#!/usr/bin/env python3
import os
import sys
import threading
import uuid
from pathlib import Path
from flask import Flask, request, jsonify
import torch
import torchaudio
import torchaudio.functional
import soundfile as sf
import tqdm
from stable_audio_tools.inference.generation import generate_diffusion_cond
from stable_audio_tools.models.utils import load_ckpt_state_dict
from stable_audio_tools.models.factory import create_model_from_config
import json

app = Flask(__name__)

# Global state
model = None
sample_rate = 44100
device = "mps" if torch.backends.mps.is_available() else ("cuda" if torch.cuda.is_available() else "cpu")
jobs = {}  # job_id -> {"state": "running"|"done"|"error", "progress": float, "output_path": str, "error": str}
_thread_local = threading.local()

# Patch tqdm to intercept progress
original_tqdm_init = tqdm.tqdm.__init__
original_tqdm_update = tqdm.tqdm.update

def hooked_tqdm_init(self, *args, **kwargs):
    original_tqdm_init(self, *args, **kwargs)
    self._job_id = getattr(_thread_local, 'job_id', None)

def hooked_tqdm_update(self, n=1):
    original_tqdm_update(self, n)
    if hasattr(self, '_job_id') and self._job_id and self._job_id in jobs:
        total = self.total if self.total else 100
        jobs[self._job_id]["progress"] = self.n / max(1, total)

tqdm.tqdm.__init__ = hooked_tqdm_init
tqdm.tqdm.update = hooked_tqdm_update

def load_model():
    global model, sample_rate, device
    print(f"Loading Foundation 1 model to {device}...", flush=True)
    try:
        amlp_dir = Path.home() / ".amlp"
        models_dir = amlp_dir / "models" / "Foundation-1"
        config_path = models_dir / "model_config.json"
        st_path = models_dir / "Foundation_1.safetensors"
        
        with open(config_path) as f:
            model_config = json.load(f)
            
        model = create_model_from_config(model_config)
        # load weights
        sd = load_ckpt_state_dict(str(st_path))
        if "state_dict" in sd:
            sd = sd["state_dict"]
        model.load_state_dict(sd)
        
        if model_config.get("model_half", True):
            model = model.half()
            
        model = model.to(device)
        model.eval()
        
        if hasattr(model, 'sample_rate'):
            sample_rate = model.sample_rate
        else:
            sample_rate = model_config.get("sample_rate", 44100)
            
        print("Model loaded successfully.", flush=True)
    except Exception as e:
        print(f"Failed to load model: {e}", file=sys.stderr)
        sys.exit(1)

@app.route("/health", methods=["GET"])
def health():
    if model is None:
        return jsonify({"status": "loading", "progress": 0.0}), 200
    return jsonify({"status": "ready", "progress": 1.0}), 200

def generate_worker(job_id, params):
    _thread_local.job_id = job_id
    try:
        steps = params.get("steps", 100)
        cfg_scale = params.get("cfgScale", 6.0)
        prompt = params.get("prompt", "")
        seed = params.get("seed", -1)
        seconds_total = params.get("seconds_total", 8.0)
        sample_size = int(seconds_total * sample_rate)
        
        bpm = params.get("bpm", 120)
        bars = params.get("bars", 4)
        musical_key = params.get("key", "")
        
        # Build a musically-aware prompt if not already complex
        extra_info = []
        if bpm and f"{bpm}bpm" not in prompt.lower() and "bpm" not in prompt.lower():
            extra_info.append(f"{bpm}bpm")
        if musical_key and musical_key not in prompt:
            extra_info.append(f"key of {musical_key}")
        if bars and "bar" not in prompt.lower():
            extra_info.append(f"{bars} bars")
            
        if extra_info:
            prompt = f"{prompt}, {', '.join(extra_info)}"

        conditioning = [{
            "prompt": prompt,
            "seconds_start": 0,
            "seconds_total": seconds_total,
            "bpm": bpm,
            "bars": bars
        }]
        
        output = generate_diffusion_cond(
            model,
            steps=steps,
            cfg_scale=cfg_scale,
            conditioning=conditioning,
            sample_size=sample_size,
            seed=seed,
            device=device,
            batch_size=1
        )
        
        # output is shape (1, channels, length)
        from einops import rearrange
        output = rearrange(output, "b d n -> d (b n)").detach().cpu().float()
        
        # EXACT LENGTH: Trim or pad to exactly sample_size
        if output.shape[1] > sample_size:
            output = output[:, :sample_size]
        elif output.shape[1] < sample_size:
            padding = torch.zeros((output.shape[0], sample_size - output.shape[1]))
            output = torch.cat([output, padding], dim=1)
            
        outs_dir = Path.home() / ".amlp" / "generated"
        outs_dir.mkdir(parents=True, exist_ok=True)
        out_file = outs_dir / f"ai_loop_{job_id}.wav"
        
        # Clamp output to prevent clipping
        output = output.clamp(-1.0, 1.0)
        sf.write(str(out_file), output.numpy().T, sample_rate)
        
        jobs[job_id]["output_path"] = str(out_file)
        jobs[job_id]["state"] = "done"
        jobs[job_id]["progress"] = 1.0
        
    except Exception as e:
        import traceback
        traceback.print_exc()
        jobs[job_id]["state"] = "error"
        jobs[job_id]["error"] = str(e)

def style_transfer_worker(job_id, params):
    _thread_local.job_id = job_id
    try:
        steps             = params.get("steps", 100)
        cfg_scale         = params.get("cfgScale", 6.0)
        prompt            = params.get("prompt", "")
        seed              = params.get("seed", -1)
        seconds_total     = params.get("seconds_total", 8.0)
        sample_size       = int(seconds_total * sample_rate)
        variation_strength = float(params.get("variation_strength", 0.7))
        source_path       = params.get("source_audio_path", "")

        bpm         = params.get("bpm", 120)
        bars        = params.get("bars", 4)
        musical_key = params.get("key", "")

        # Load source audio via soundfile (avoids torchaudio backend issues)
        audio_np, source_sr = sf.read(source_path, always_2d=True)
        # soundfile gives (frames, channels); convert to (channels, frames)
        source_audio = torch.from_numpy(audio_np.T).float()
        if source_sr != sample_rate:
            source_audio = torchaudio.functional.resample(source_audio, source_sr, sample_rate)

        # Trim or pad to exact sample_size
        if source_audio.shape[1] > sample_size:
            source_audio = source_audio[:, :sample_size]
        elif source_audio.shape[1] < sample_size:
            padding = torch.zeros((source_audio.shape[0], sample_size - source_audio.shape[1]))
            source_audio = torch.cat([source_audio, padding], dim=1)

        # Match model dtype and device
        model_dtype = next(model.parameters()).dtype
        source_audio = source_audio.to(device=device, dtype=model_dtype)

        # Build conditioning prompt (same enrichment as generate_worker)
        extra_info = []
        if bpm and f"{bpm}bpm" not in prompt.lower() and "bpm" not in prompt.lower():
            extra_info.append(f"{bpm}bpm")
        if musical_key and musical_key not in prompt:
            extra_info.append(f"key of {musical_key}")
        if bars and "bar" not in prompt.lower():
            extra_info.append(f"{bars} bars")
        if extra_info:
            prompt = f"{prompt}, {', '.join(extra_info)}"

        conditioning = [{
            "prompt": prompt,
            "seconds_start": 0,
            "seconds_total": seconds_total,
            "bpm": bpm,
            "bars": bars
        }]

        output = generate_diffusion_cond(
            model,
            steps=steps,
            cfg_scale=cfg_scale,
            conditioning=conditioning,
            sample_size=sample_size,
            seed=seed,
            device=device,
            batch_size=1,
            init_audio=(sample_rate, source_audio),
            init_noise_level=variation_strength
        )

        from einops import rearrange
        output = rearrange(output, "b d n -> d (b n)").detach().cpu().float()

        if output.shape[1] > sample_size:
            output = output[:, :sample_size]
        elif output.shape[1] < sample_size:
            padding = torch.zeros((output.shape[0], sample_size - output.shape[1]))
            output = torch.cat([output, padding], dim=1)

        outs_dir = Path.home() / ".amlp" / "generated"
        outs_dir.mkdir(parents=True, exist_ok=True)
        out_file = outs_dir / f"ai_vary_{job_id}.wav"

        output = output.clamp(-1.0, 1.0)
        sf.write(str(out_file), output.numpy().T, sample_rate)

        jobs[job_id]["output_path"] = str(out_file)
        jobs[job_id]["state"] = "done"
        jobs[job_id]["progress"] = 1.0

    except Exception as e:
        import traceback
        traceback.print_exc()
        jobs[job_id]["state"] = "error"
        jobs[job_id]["error"] = str(e)


@app.route("/style_transfer", methods=["POST"])
def style_transfer():
    params = request.json
    job_id = str(uuid.uuid4())
    jobs[job_id] = {"state": "running", "progress": 0.0}
    thread = threading.Thread(target=style_transfer_worker, args=(job_id, params))
    thread.start()
    return jsonify({"job_id": job_id})


@app.route("/generate", methods=["POST"])
def generate():
    params = request.json
    job_id = str(uuid.uuid4())
    jobs[job_id] = {"state": "running", "progress": 0.0}
    
    thread = threading.Thread(target=generate_worker, args=(job_id, params))
    thread.start()
    
    return jsonify({"job_id": job_id})

@app.route("/status/<job_id>", methods=["GET"])
def status(job_id):
    if job_id not in jobs:
        return jsonify({"error": "Job not found"}), 404
    return jsonify(jobs[job_id])

if __name__ == "__main__":
    load_thread = threading.Thread(target=load_model)
    load_thread.start()
    app.run(host="127.0.0.1", port=8976)

#!/usr/bin/env python3
import os
import subprocess
import sys
from pathlib import Path

def main():
    amlp_dir = Path.home() / ".amlp"
    venv_dir = amlp_dir / "ai-venv"
    models_dir = amlp_dir / "models" / "Foundation-1"
    ready_file = amlp_dir / "ai-ready"

    if ready_file.exists() and venv_dir.exists():
        print("PROGRESS:0.99:AI Environment already set up.", flush=True)
        print("FINISHED_SETUP", flush=True)
        sys.exit(0)

    amlp_dir.mkdir(parents=True, exist_ok=True)
    models_dir.mkdir(parents=True, exist_ok=True)

    if not venv_dir.exists():
        print("PROGRESS:0.05:Creating virtual environment...", flush=True)
        try:
            subprocess.run([sys.executable, "-m", "venv", str(venv_dir)], check=True)
        except Exception as e:
            print(f"ERROR:Failed to create venv: {e}")
            sys.exit(1)
    
    pip_exe = venv_dir / "bin" / "pip"
    if not pip_exe.exists():
        print("ERROR:Failed to find pip in venv", file=sys.stderr)
        sys.exit(1)

    print("PROGRESS:0.15:Installing PyTorch & Stable Audio Tools...", flush=True)
    try:
        subprocess.run([str(pip_exe), "install", "--upgrade", "pip", "setuptools", "wheel"], check=True, stdout=subprocess.DEVNULL)
        subprocess.run([str(pip_exe), "install", "stable-audio-tools", "k-diffusion", "encodec", "local-attention", "auraloss", "einops-exts", "ema-pytorch", "laion-clap", "prefigure", "pytorch-lightning", "PyWavelets", "sentencepiece", "torchmetrics", "vector-quantize-pytorch", "wandb", "webdataset", "jsonschema", "alias-free-torch", "scikit-image", "--no-deps"], check=True)
        subprocess.run([str(pip_exe), "install", "torch", "torchaudio", "einops", "safetensors", "transformers", "descript-audio-codec", "julius", "huggingface_hub", "flask", "werkzeug", "tqdm", "v-diffusion-pytorch", "numpy", "scipy"], check=True)
    except subprocess.CalledProcessError as e:
        print(f"ERROR:Failed to install deps: {e}", file=sys.stderr)
        sys.exit(1)
    
    py_exe = venv_dir / "bin" / "python"
    
    print("PROGRESS:0.50:Downloading Foundation 1 model (~2.3GB)...", flush=True)
    download_script = """
import os
from huggingface_hub import snapshot_download

model_id = "RoyalCities/Foundation-1"
local_dir = "{local_dir}"

snapshot_download(repo_id=model_id, local_dir=local_dir)
"""
    download_script = download_script.replace("{local_dir}", str(models_dir))
    dl_script_path = amlp_dir / "temp_dl.py"
    dl_script_path.write_text(download_script)
    
    try:
        subprocess.run([str(py_exe), str(dl_script_path)], check=True)
    except subprocess.CalledProcessError as e:
        print(f"ERROR:Failed to download model: {e}", file=sys.stderr)
        sys.exit(1)
    finally:
        if dl_script_path.exists():
            dl_script_path.unlink()

    print("PROGRESS:1.0:Setup complete.", flush=True)
    ready_file.touch()
    print("FINISHED_SETUP", flush=True)

if __name__ == "__main__":
    main()

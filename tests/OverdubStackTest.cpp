#include <juce_core/juce_core.h>
#include "core/OverdubStack.h"

class OverdubStackTest : public juce::UnitTest
{
public:
    OverdubStackTest() : UnitTest ("OverdubStack", "AMLP") {}

    void runTest() override
    {
        beginTest ("Empty stack");
        {
            OverdubStack stack;
            expect (! stack.canUndo());
            expect (! stack.canRedo());
            expectEquals (stack.getNumActiveLayers(), 0);
            expect (stack.getBaseLayer() == juce::File());
            expect (stack.getTopLayer() == juce::File());
            expect (stack.getActiveLayers().empty());
        }

        beginTest ("Push and pop layers");
        {
            OverdubStack stack;
            juce::File f1 ("/tmp/layer1.wav");
            juce::File f2 ("/tmp/layer2.wav");
            juce::File f3 ("/tmp/layer3.wav");

            stack.pushLayer (f1);
            expectEquals (stack.getNumActiveLayers(), 1);
            expect (stack.getBaseLayer() == f1);
            expect (stack.getTopLayer() == f1);
            expect (stack.canUndo());
            expect (! stack.canRedo());

            stack.pushLayer (f2);
            stack.pushLayer (f3);
            expectEquals (stack.getNumActiveLayers(), 3);
            expect (stack.getBaseLayer() == f1);
            expect (stack.getTopLayer() == f3);

            auto layers = stack.getActiveLayers();
            expectEquals (static_cast<int> (layers.size()), 3);
            expect (layers[0] == f1);
            expect (layers[1] == f2);
            expect (layers[2] == f3);
        }

        beginTest ("Undo (pop)");
        {
            OverdubStack stack;
            juce::File f1 ("/tmp/layer1.wav");
            juce::File f2 ("/tmp/layer2.wav");

            stack.pushLayer (f1);
            stack.pushLayer (f2);

            auto popped = stack.popLayer();
            expect (popped == f2);
            expectEquals (stack.getNumActiveLayers(), 1);
            expect (stack.getTopLayer() == f1);
            expect (stack.canUndo());
            expect (stack.canRedo());
        }

        beginTest ("Redo (restore)");
        {
            OverdubStack stack;
            juce::File f1 ("/tmp/layer1.wav");
            juce::File f2 ("/tmp/layer2.wav");

            stack.pushLayer (f1);
            stack.pushLayer (f2);
            stack.popLayer();

            auto restored = stack.restoreLayer();
            expect (restored == f2);
            expectEquals (stack.getNumActiveLayers(), 2);
            expect (stack.getTopLayer() == f2);
            expect (! stack.canRedo());
        }

        beginTest ("Push after undo clears redo history");
        {
            OverdubStack stack;
            juce::File f1 ("/tmp/layer1.wav");
            juce::File f2 ("/tmp/layer2.wav");
            juce::File f3 ("/tmp/layer3.wav");

            stack.pushLayer (f1);
            stack.pushLayer (f2);
            stack.popLayer(); // undo f2

            stack.pushLayer (f3); // this should clear f2 from redo
            expect (! stack.canRedo());
            expectEquals (stack.getNumActiveLayers(), 2);
            expectEquals (stack.getTotalLayers(), 2);
            expect (stack.getTopLayer() == f3);
        }

        beginTest ("Pop on empty returns empty file");
        {
            OverdubStack stack;
            auto result = stack.popLayer();
            expect (result == juce::File());
        }

        beginTest ("Restore with nothing to redo returns empty file");
        {
            OverdubStack stack;
            juce::File f1 ("/tmp/layer1.wav");
            stack.pushLayer (f1);

            auto result = stack.restoreLayer();
            expect (result == juce::File());
        }

        beginTest ("Clear removes everything");
        {
            OverdubStack stack;
            stack.pushLayer (juce::File ("/tmp/a.wav"));
            stack.pushLayer (juce::File ("/tmp/b.wav"));
            stack.clear();

            expectEquals (stack.getNumActiveLayers(), 0);
            expectEquals (stack.getTotalLayers(), 0);
            expect (! stack.canUndo());
            expect (! stack.canRedo());
        }
    }
};

static OverdubStackTest overdubStackTest;

# segModule

**Modular Code Redesign with Testing Framework (Section: Image Segmentation)**

segModule is one section of a larger effort: redesigning the codebase around clean
module boundaries and building a test framework alongside it. Image segmentation is
the section in progress. The segmentation code matters less than the structure around
it: a factory pattern, config-driven design in YAML, a generic feature model, and a
GTest suite that checks each piece fits.

`ARCHITECTURE.md` covers the design reasoning: the layering, the bagLoader seam, and
what is integrated versus stubbed. This README covers building, running, and testing
day to day.

---

## What's here

One build produces two executables:

| Target | What it is | Inputs |
|---|---|---|
| `basicseg` | The program. Loads a `.mi.gz` bag through the coworker's bagLoader, then segments it. | a YAML config + a bag file |
| `segmenter_test` | The GTest suite. Synthetic in-memory images; no bagLoader, no boost, no real data. | none (fixtures built in code) |

The shape of the code, top to bottom:

```
ConfigLoader (YAML)  ─►  SegmenterFactoryBase ─► Threshold / Canny factory
                                  │
                                  ▼
                         SegmenterBase (abstract)
                           └─ CommonSegmenter
                                ├─ ThresholdSegmenter
                                └─ CannySegmenter
                                       │
                  SegImage ──► segment() ──► Objects (each carries named features)
```

`SegImage` is the image type the segmenter runs on, a light `cv::Mat` wrapper. It
stands in for the bagLoader's image class until that class exists. `bag_adapter` is
the one file that connects the two layers. See `ARCHITECTURE.md` for the details.

---

## Adding a segmenter

The structure around the segmenters stays fixed. The inside of each one varies.

Every segmenter shares the same contract: `segment()` takes a `SegImage` and returns
a label `SegImage` plus an `Objects` collection. The factory (`factoryStyle`) builds
the right segmenter from the YAML config. A caller reads any segmenter's output the
same way.

The body of `segment()` is open. The current `ThresholdSegmenter` and
`CannySegmenter` call OpenCV directly (`cv::threshold`, `cv::Canny`,
`cv::connectedComponentsWithStats`), so much of the per-style logic is OpenCV. A
segmenter built on a different technique can look nothing like these inside. The
structure constrains the input and output types, not the method.

To add a style: subclass `CommonSegmenter`, implement `segment()`, add a factory, and
register the style name in `factoryStyle()`. The calling code changes nothing beyond
the YAML.

One gap to know: the output is universal (every segmenter returns `Objects` plus a
label image), but the input is not yet. `SegImage` is a `cv::Mat` wrapper standing in
for `ALOG::ALOG_Image` until that class exists, so today the input type is
OpenCV-specific. `bag_adapter` is the single place to swap it once `ALOG_Image`
lands. `ARCHITECTURE.md` has the full picture.

---

## Build

You need OpenCV, yaml-cpp, GTest, and boost (iostreams). `CMakeLists.txt` resolves all
four on this box.

First clean build:

```bash
cd /mnt/netapp/SECURITY/users/rnguyen/segModule/build
cmake ..
make
```

After the first build, use the rebuild loop below instead of a full `make`.

---

## Running the tests

The tests compile into one executable, `build/segmenter_test`, and you can run it
from any directory. Rebuild first so it reflects your latest changes.

### Rebuild, then run. Every time.

Editing `segmenter.cpp` or `segmenter_test.cpp` changes the source, not the compiled
binary. Run the old executable without rebuilding and you test old code. This trips
people up more than anything else.

`cmake --build` takes the build folder as an argument, so it runs from any directory
without a `cd` first:

```bash
cmake --build /mnt/netapp/SECURITY/users/rnguyen/segModule/build --target segmenter_test
```

That recompiles what changed, relinks the executable, and copies the test config next
to it. Then run the binary from wherever you are:

```bash
/mnt/netapp/SECURITY/users/rnguyen/segModule/build/segmenter_test
```

It runs from anywhere because the tests find their config fixture next to the
executable rather than in your current directory, and CMake copies `test_config.yaml`
there on every build. `build/`, the repo root, and `/tmp` all give the same result.
`ARCHITECTURE.md` has the mechanics.

### One alias for the whole loop

Add this to your `~/.bashrc` so you skip typing the path. It rebuilds, then runs:

```bash
alias segtest='cmake --build /mnt/netapp/SECURITY/users/rnguyen/segModule/build --target segmenter_test \
  && /mnt/netapp/SECURITY/users/rnguyen/segModule/build/segmenter_test'
```

Then, from any directory after a change:

```bash
segtest
```

Edit code, run `segtest`, read the results.

### GTest flags

The executable accepts the standard GTest flags:

```bash
./build/segmenter_test --gtest_list_tests              # list every test
./build/segmenter_test --gtest_filter='ConfigLoader*'  # run a subset
./build/segmenter_test --gtest_filter='*Canny*'        # match by name
```

### What passing looks like

The suite reports **11 passed, 6 failed** from any directory, and that count is
expected. The 6 failures feed unwired synthetic inputs into `segment()` (see the
inline notes in `tests/segmenter_test.cpp`). Each one fails an assertion; none of them
crash. They mark work deferred at this concept stage. A different count, or a crash,
means something changed, so check that you rebuilt before running.

---

## Running the program

`basicseg` takes a YAML config and a bag file:

```bash
cd /mnt/netapp/SECURITY/users/rnguyen/segModule
./build/basicseg tests/test_config.yaml \
    /mnt/netapp/SECURITY/users/ltang/public/TestGoogleTest/sample.mi.gz
```

The config picks the segmentation style (`Threshold` or `Canny`) and its parameters.
The factory reads the style and builds the matching segmenter. Change the style in the
YAML and rerun; no recompile needed.

---

## Where to go next

- [`ARCHITECTURE.md`](ARCHITECTURE.md): the design. Layers, the bagLoader seam, what
  is integrated versus stubbed, and the open decisions for review.
- `tests/segmenter_test.cpp`: the suite, with inline notes on the expected failures
  and what each fixture exercises.

# segModule — Architecture

This document explains how the segmenter relates to the bagLoader module, what is
genuinely integrated vs. stubbed, and the design decisions worth knowing for review.

## Layers and the seam

The bagLoader and the segmenter are **separate layers** connected by a single
**adapter** (`bag_adapter`). Each layer is deliberately ignorant of the other's
dependencies:

```
  bagLoader layer            knows: .mi.gz, gzip, boost 3-D volumes
   (ltang's module)                 (does NOT know about OpenCV)
        │
        │   ── bag_adapter ──   ← the seam: 3-D volume slice → 2-D cv::Mat
        ▼
  segmenter layer            knows: cv::Mat, OpenCV, SegImage
   (this module)                    (does NOT know about .mi.gz or boost)
```

This separation is intentional: the loader should not depend on OpenCV, and the
segmenter should not depend on boost / the `.mi.gz` format. The adapter is the one
place those two worlds meet.

## Two run paths

`basicseg` (the program) uses the real bagLoader. The unit tests do not — they use
synthetic in-memory images and never touch the bagLoader or boost.

```
  basicseg (PROGRAM):
    main.cpp ─ load(path) ─► seg_image.cpp ─► bag_adapter.cpp ─► ALOG::Bag
                                                  (the BRIDGE)     (file_io.h)
                                                                   reads .mi.gz
                                                                   → boost 3-D volume
                              SegImage ◄── slice + normalize ◄── cv::Mat
                                 │
                                 ▼
                       segmenter->segment() ─► Objects

  segmenter_test (UNIT TESTS):
    synthetic cv::Mat (cv::rectangle, built in code)
         ▼
    SegImage ─► segmenter->segment() ─► Objects
    ★ no bagLoader, no boost, no coworker code
```

## What is / isn't integrated with the bagLoader

| Coworker's code (`ltang/.../TestGoogleTest`) | Used? | Notes |
|---|---|---|
| `ALOG::Bag` + `file_io.h` `ReadMIGZImage` | ✅ yes | The real `.mi.gz` decompress + 3-D parse. Compiled into `basicseg`. |
| `ALOG::ALOG_Image` | ❌ no | Empty, unimplemented stub (no storage, no defined methods). |
| `ALOG::Bag::Save/Load`, `Load_Type`, their `main.cpp` | ❌ no | Not needed here. |

## The image type: `SegImage` (and the `ALOG_Image` placeholder note)

`segment()` takes `SegImage` — a lightweight `cv::Mat`-backed image wrapper defined
in `seg_image.hpp`. It exists because the bagLoader's intended image class,
`ALOG::ALOG_Image`, is currently an **empty stub** and cannot be used as an image
type yet.

Why a separate type rather than `ALOG::ALOG_Image`:
- C++'s One Definition Rule forbids two different definitions of `ALOG::ALOG_Image`
  in one program. Truly adopting it would require implementing it **inside** (or
  forking) the coworker's shared module — a change to coordinate with them, not to
  make unilaterally.
- Keeping `SegImage` local means the segmenter and its tests don't depend on the
  bagLoader's headers or boost.

Accurate framing: *the segmenter operates on an image abstraction; the intended type
is `ALOG::ALOG_Image`, but until that is built out the segmenter uses `SegImage`, and
`bag_adapter` is the single swap point.*

## Class structure (unchanged by the integration)

```
SegmenterBase (abstract)
   └─ CommonSegmenter (shared: toGray, findObjects, convert*)
        ├─ ThresholdSegmenter
        └─ CannySegmenter

SegmenterFactoryBase ─► ThresholdSegmenterFactory / CannySegmenterFactory
ConfigLoader,  Object / Objects
SegImage   ← a data type passed THROUGH segment(); not part of the hierarchy
```

## What's fixed vs. what varies per segmenter

The structure around the segmenters is fixed. The inside of each segmenter varies.

**Fixed, every segmenter shares it:**
- The `segment()` contract: a `SegImage` in, a label `SegImage` plus an `Objects`
  collection out. Same signature for every style.
- The factory path: `SegmenterFactoryBase::factoryStyle(name)` returns the derived
  factory, which builds the segmenter from `ConfigLoader`.
- The output model: results come back as `Objects`, each carrying named features. A
  caller reads any segmenter's output the same way.

**Varies, per style:**
- The body of `segment()`. The current `ThresholdSegmenter` and `CannySegmenter` call
  OpenCV directly (`cv::threshold`, `cv::Canny`, `cv::connectedComponentsWithStats`),
  so much of the per-style logic is OpenCV today. A different technique (watershed,
  contour-based, a learned model) can look nothing like these inside. The structure
  constrains the input and output types, not the method.

**Adding a style:** subclass `CommonSegmenter`, implement `segment()`, add a factory,
and register the style name in `factoryStyle()`. The config and the calling code need
no change beyond the YAML style name.

**Universal output, not-yet-universal input.** Output is universal today: every
segmenter returns `Objects` plus a label image. Input is not universal yet.
`segment()` takes `SegImage`, a `cv::Mat` wrapper standing in for `ALOG::ALOG_Image`.
Until `ALOG_Image` exists, the input type is OpenCV-specific. When it lands, it
becomes the shared input type and `bag_adapter` is the single swap point (see the
image-type section above).

## Open decisions (flag in review)

- **3-D → 2-D:** the adapter takes the middle z-slice and normalizes 16-bit → 8-bit.
  Which slice (or whether to process all slices) is a product decision, not final.
- **Unifying the image type:** if the team implements `ALOG_Image` for real, decide
  whether it becomes the shared image type across both modules (would require the
  bagLoader to take on an image representation the segmenter can consume).

## Build & run

```bash
# From the repo root. basicseg needs the bagLoader module (not in this repo),
# so pass its path; the test target builds without it.
cmake -S . -B build -DBAGLOADER_DIR=<path-to-bagloader>
cmake --build build

# Program: segment a real bag via the bagLoader
./build/basicseg tests/test_config.yaml <path-to-bag>.mi.gz

# Unit tests: synthetic inputs, per-test pass/fail reporting
#   runs from any directory — see the working-directory note below
./build/segmenter_test
```

For the full day-to-day workflow — the rebuild→run loop, running the executable
from any directory, and the alias the team uses — see [`README.md`](README.md).

### Working-directory independence (how the fixture is found)

The tests need a config fixture (`tests/test_config.yaml`). Rather than open it by a
relative path — which would tie passing to the launch directory — the test resolves
it **relative to the executable itself**: `configPath()` in `segmenter_test.cpp`
reads `/proc/self/exe` (the kernel's symlink to the running binary) and looks for
`test_config.yaml` beside it. CMake stages a copy there on every build via a
`POST_BUILD` step (`copy_if_different`, see `CMakeLists.txt`).

The result: `segmenter_test` passes identically whether launched from the repo root,
from `build/`, or from anywhere else — and so does `ctest`, which runs from the build
dir. Edit the fixture and rebuild; the copy refreshes automatically.

Current test status: **11 passed / 6 failed** (same from any directory). The 6
failures are expected — those tests feed unwired/synthetic-but-unassigned inputs
into `segment()` (see the inline notes in `tests/segmenter_test.cpp`); they are
honest failures, not crashes.

/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstdint>

#include "RenderScriptToolkit.h"
#include "TaskProcessor.h"
#include "Utils.h"

#define LOG_TAG "renderscript.toolkit.Threshold"

namespace renderscript {

    class ThresholdTask : public Task {
        const uchar4 *mIn;
        uchar4 *mOut;
        const uchar mThreshold;
        bool mBinary;

        // Process a 2D tile of the overall work. threadIndex identifies which thread does the work.
        void processData(int threadIndex, size_t startX, size_t startY, size_t endX,
                         size_t endY) override;

    public:
        ThresholdTask(const uint8_t *input, uint8_t *output, size_t sizeX, size_t sizeY,
                      const uint8_t threshold, bool binary,
                      const Restriction *restriction)
                : Task{sizeX, sizeY, 4, true, restriction},
                  mIn{reinterpret_cast<const uchar4 *>(input)},
                  mOut{reinterpret_cast<uchar4 *>(output)},
                  mThreshold{threshold},
                  mBinary{binary} {}
    };

    void
    ThresholdTask::processData(int /* threadIndex */, size_t startX, size_t startY, size_t endX,
                               size_t endY) {
        for (size_t y = startY; y < endY; y++) {
            size_t offset = mSizeX * y + startX;
            const uchar4 *in = mIn + offset;
            uchar4 *out = mOut + offset;
            for (size_t x = startX; x < endX; x++) {
                auto v = *in;
                auto average = (v.x + v.y + v.z) / 3;
                if (average > mThreshold && !mBinary){
                    *out = uchar4{v.x, v.y, v.z, v.w};
                } else if (average > mThreshold && mBinary){
                    *out = uchar4{255, 255, 255, v.w};
                } else {
                    *out = uchar4{0, 0, 0, v.w};
                }
                in++;
                out++;
            }
        }
    }

    void RenderScriptToolkit::threshold(const uint8_t *input, uint8_t *output, size_t sizeX,
                                        size_t sizeY,
                                        const uint8_t threshold, bool binary,
                                        const Restriction *restriction) {
#ifdef ANDROID_RENDERSCRIPT_TOOLKIT_VALIDATE
        if (!validRestriction(LOG_TAG, sizeX, sizeY, restriction)) {
            return;
        }
#endif

        ThresholdTask task(input, output, sizeX, sizeY, threshold, binary, restriction);
        processor->doTask(&task);
    }

}  // namespace renderscript

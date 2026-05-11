/*
 * Copyright (C) 2021 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "Vibrator.h"

#include <android-base/logging.h>

#include <fstream>
#include <thread>
#include <unistd.h>

namespace aidl {
namespace android {
namespace hardware {
namespace vibrator {

/*
 * Write value to sysfs node
 */
template <typename T>
static ndk::ScopedAStatus writeNode(
        const std::string& path,
        const T& value) {

    std::ofstream node(path);

    if (!node.is_open()) {
        LOG(ERROR) << "Failed to open: " << path;

        return ndk::ScopedAStatus::fromExceptionCode(
                EX_ILLEGAL_STATE);
    }

    node << value << std::endl;

    if (!node.good()) {
        LOG(ERROR) << "Failed to write: " << value;

        return ndk::ScopedAStatus::fromExceptionCode(
                EX_ILLEGAL_STATE);
    }

    return ndk::ScopedAStatus::ok();
}

static bool nodeExists(const std::string& path) {
    return access(path.c_str(), F_OK) == 0;
}

Vibrator::Vibrator() {
    mIsTimedOutVibrator =
            nodeExists(VIBRATOR_TIMEOUT_PATH);

    LOG(INFO) << "Timed output vibrator: "
              << mIsTimedOutVibrator;
}

ndk::ScopedAStatus Vibrator::getCapabilities(
        int32_t* _aidl_return) {

    *_aidl_return =
            IVibrator::CAP_ON_CALLBACK |
            IVibrator::CAP_PERFORM_CALLBACK;

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::off() {
    return activate(0);
}

ndk::ScopedAStatus Vibrator::on(
        int32_t timeoutMs,
        const std::shared_ptr<IVibratorCallback>& callback) {

    auto status = activate(timeoutMs);

    if (!status.isOk()) {
        return status;
    }

    if (callback != nullptr) {
        std::thread([callback, timeoutMs] {
            usleep(timeoutMs * 1000);

            if (!callback->onComplete().isOk()) {
                LOG(ERROR) << "Failed to notify callback";
            }
        }).detach();
    }

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::perform(
        Effect effect,
        EffectStrength /*strength*/,
        const std::shared_ptr<IVibratorCallback>& callback,
        int32_t* _aidl_return) {

    ndk::ScopedAStatus status;

    uint32_t ms = effectToMs(effect, &status);

    if (!status.isOk()) {
        return status;
    }

    status = activate(ms);

    if (!status.isOk()) {
        return status;
    }

    *_aidl_return = ms;

    if (callback != nullptr) {
        std::thread([callback, ms] {
            usleep(ms * 1000);

            if (!callback->onComplete().isOk()) {
                LOG(ERROR) << "Failed to notify callback";
            }
        }).detach();
    }

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::getSupportedEffects(
        std::vector<Effect>* _aidl_return) {

    *_aidl_return = {
            Effect::CLICK,
            Effect::DOUBLE_CLICK,
            Effect::TICK,
            Effect::TEXTURE_TICK,
            Effect::HEAVY_CLICK
    };

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::setAmplitude(float) {
    return ndk::ScopedAStatus::fromExceptionCode(
            EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Vibrator::setExternalControl(bool) {
    return ndk::ScopedAStatus::fromExceptionCode(
            EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Vibrator::getCompositionDelayMax(
        int32_t*) {

    return ndk::ScopedAStatus::fromExceptionCode(
            EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Vibrator::getCompositionSizeMax(
        int32_t*) {

    return ndk::ScopedAStatus::fromExceptionCode(
            EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Vibrator::getSupportedPrimitives(
        std::vector<CompositePrimitive>*) {

    return ndk::ScopedAStatus::fromExceptionCode(
            EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Vibrator::getPrimitiveDuration(
        CompositePrimitive,
        int32_t*) {

    return ndk::ScopedAStatus::fromExceptionCode(
            EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Vibrator::compose(
        const std::vector<CompositeEffect>&,
        const std::shared_ptr<IVibratorCallback>&) {

    return ndk::ScopedAStatus::fromExceptionCode(
            EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Vibrator::getSupportedAlwaysOnEffects(
        std::vector<Effect>*) {

    return ndk::ScopedAStatus::fromExceptionCode(
            EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Vibrator::alwaysOnEnable(
        int32_t,
        Effect,
        EffectStrength) {

    return ndk::ScopedAStatus::fromExceptionCode(
            EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Vibrator::alwaysOnDisable(
        int32_t) {

    return ndk::ScopedAStatus::fromExceptionCode(
            EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Vibrator::getResonantFrequency(
        float*) {

    return ndk::ScopedAStatus::fromExceptionCode(
            EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Vibrator::getQFactor(
        float*) {

    return ndk::ScopedAStatus::fromExceptionCode(
            EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Vibrator::getFrequencyResolution(
        float*) {

    return ndk::ScopedAStatus::fromExceptionCode(
            EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Vibrator::getFrequencyMinimum(
        float*) {

    return ndk::ScopedAStatus::fromExceptionCode(
            EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Vibrator::getBandwidthAmplitudeMap(
        std::vector<float>*) {

    return ndk::ScopedAStatus::fromExceptionCode(
            EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Vibrator::getPwlePrimitiveDurationMax(
        int32_t*) {

    return ndk::ScopedAStatus::fromExceptionCode(
            EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Vibrator::getPwleCompositionSizeMax(
        int32_t*) {

    return ndk::ScopedAStatus::fromExceptionCode(
            EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Vibrator::getSupportedBraking(
        std::vector<Braking>*) {

    return ndk::ScopedAStatus::fromExceptionCode(
            EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Vibrator::composePwle(
        const std::vector<PrimitivePwle>&,
        const std::shared_ptr<IVibratorCallback>&) {

    return ndk::ScopedAStatus::fromExceptionCode(
            EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Vibrator::activate(
        uint32_t timeoutMs) {

    std::lock_guard<std::mutex> lock(mMutex);

    if (!mIsTimedOutVibrator) {
        return ndk::ScopedAStatus::fromExceptionCode(
                EX_UNSUPPORTED_OPERATION);
    }

    return writeNode(
            VIBRATOR_TIMEOUT_PATH,
            timeoutMs);
}

uint32_t Vibrator::effectToMs(
        Effect effect,
        ndk::ScopedAStatus* status) {

    *status = ndk::ScopedAStatus::ok();

    switch (effect) {
        case Effect::CLICK:
            return 30;

        case Effect::DOUBLE_CLICK:
            return 60;

        case Effect::TICK:
        case Effect::TEXTURE_TICK:
            return 20;

        case Effect::HEAVY_CLICK:
            return 80;

        default:
            *status =
                ndk::ScopedAStatus::fromExceptionCode(
                        EX_UNSUPPORTED_OPERATION);

            return 0;
    }
}

} // namespace vibrator
} // namespace hardware
} // namespace android
} // namespace aidl
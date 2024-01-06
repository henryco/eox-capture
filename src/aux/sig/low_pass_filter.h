//
// Created by henryco on 1/6/24.
//

#ifndef STEREOX_LOW_PASS_FILTER_H
#define STEREOX_LOW_PASS_FILTER_H

namespace eox::sig {

    /**
     * The term "low pass filter" in this context might be slightly misleading if you're used to the conventional
     * meaning from electronics or signal processing, where it refers to a filter that passes signals with a frequency
     * lower than a certain cutoff frequency and attenuates signals with frequencies higher than that cutoff frequency.
     *
     * There, a low pass filter effectively smooths out rapid changes in a signal,
     * thereby reducing its high-frequency components.
     */
    class LowPassFilter {
    private:
        float last_value;
        bool initialized;

    public:
        LowPassFilter();

        /**
         * @param value any value
         * @param alpha should be in range [0,1]
         * @return filtered value
         */
        float filter(float value, float alpha);

        [[nodiscard]] float getLastValue() const;
    };

} // eox

#endif //STEREOX_LOW_PASS_FILTER_H

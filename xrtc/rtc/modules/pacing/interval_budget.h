//
// Created by faker on 2023/7/2.
//

#ifndef XRTCSDK_INTERVAL_BUDGET_H
#define XRTCSDK_INTERVAL_BUDGET_H


#include <stdint.h>

namespace xrtc {

    class IntervalBudget {
    public:
        IntervalBudget(int initial_target_bitrate_kbps,
                       bool can_build_up_underuse = false);
        ~IntervalBudget();

        void set_target_bitrate_kbps(int target_bitrate_kbps);
        void IncreaseBudget(int64_t elapsed_time);
        void UseBudget(size_t bytes);
        size_t bytes_remaining();

    private:
        int target_bitrate_kbps_;
        int64_t max_bytes_in_budget_;
        int64_t bytes_remaining_;
        bool can_build_up_underuse_;
    };

} // namespace xrtc



#endif //XRTCSDK_INTERVAL_BUDGET_H

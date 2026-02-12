#include "../../include/core/app_data.hpp"

void AppData::stop(std::shared_ptr<BonsaiData> data) {
    {
        std::lock_guard<std::mutex> lock(data->cv_mutex);
        data->stop = true;
    }
    data->cv.notify_all();
}
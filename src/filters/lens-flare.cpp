#include "filter.hpp"
#include "frame.hpp"
#include "logger.hpp"
#include "utils.hpp"
#include "mat-cache.hpp"

#include <cmath>
#include <map>
#include <vector>

using namespace cv;
using namespace CCPlus;

static const std::vector<float> distance = {
    1.0f, 1.0f, -0.5f, -0.25f, 1.25f, 0.5, 0.6
};

static const std::vector<float> scale = {
    3.0, 1.0f, 0.75, 0.25, 0.5, 0.35, 0.25
};

/*
 * A beta version lens flare
 */
CCPLUS_FILTER(lens_flare) {
    if (parameters.size() == 0)
        return;
    if (parameters.size() < 4) {
        log(logERROR) << "Not enough parameters for lens flare";
    }
    if (frame.getImage().empty() || frame.getWidth() < 10 || 
            frame.getHeight() < 10) 
        return;
    int type = parameters[3];
    float opacity = parameters[2]; // 0.0 ~ 1.0
    Point2f light(parameters[0], parameters[1]);
    Point2f center(frame.getWidth() / 2.0,
            frame.getHeight() / 2.0);

    Point2f v = light - center;
    // Normalize
    float length = norm(v);
    v *= 1.0 / length;

    //Mat mask(frame.getHeight(), frame.getWidth(), 
    //        CV_8UC4, {0, 0, 0, 255});

    int flareSize = std::max(frame.getHeight(), frame.getWidth()) / 4;

    for (int i = 0; i < distance.size(); i++) {
        int fsize = flareSize * scale[i];
        Point2f p = center + v * length * distance[i];
        p -= Point2f(fsize / 2, fsize / 2);
        Mat flare = MatCache::get("res/" + toString(i) + ".jpg", [i] () {
            return imread("res/" + toString(i) + ".jpg", CV_LOAD_IMAGE_UNCHANGED);
        });
        auto access = [&flare, fsize] (Point2f t) {
            float tmp_x = t.x * 1.0 / fsize * 360.0;
            float tmp_y = t.y * 1.0 / fsize * 360.0;
            int cols = flare.cols;
            int i = std::round(tmp_y), j = std::round(tmp_x);
            return flare.ptr<Vec3b>(0)[i * cols + j];
            //return flare.at<Vec3b>(std::round(tmp_y), std::round(tmp_x));
        };
        Vec4b* ptr = frame.getImage().ptr<Vec4b>(0);
        for (int x = 0; x < fsize; x++) {
            for (int y = 0; y < fsize; y++) {
                Vec3b col = access(Point2f(x, y));
                if (y + p.y < 0 || x + p.x < 0 ||
                    y + p.y >= frame.getHeight() || 
                    x + p.x >= frame.getWidth())
                    continue;
                //Vec4b& m = frame.getImage().at<Vec4b>(y + p.y, x + p.x);
                int cols = frame.getImage().cols;
                Vec4b& m = ptr[(y + (int)p.y) * cols + x + (int)p.x];
                float oalpha = m[3] / 255.0;
                float nalpha = oalpha + opacity - oalpha * opacity;
                for (int k = 0; k < 3; k++) {
                    float tmp = m[k] * oalpha + col[k] * opacity;
                    m[k] = between<int>(tmp / nalpha, 0, 255);
                }
                m[3] = nalpha * 255.0;
            }
        }
    }
    //imwrite("tmp/mask.png", mask);
    //Frame ret(mask);
    //ret.mergeFrame(frame, 1);
    //frame = ret;
}

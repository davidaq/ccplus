#include "filter.hpp"
#include <cmath>

using namespace cv;

// Using intersection to check if point in polygon
CCPLUS_FILTER(mask) {
    int np = parameters.size() / 2;
    std::vector<Point2f> ps;
    for (int i = 0; i < np; i++)
        ps.push_back(Point2f(parameters[i * 2], parameters[i * 2 + 1]));

    Mat& image = frame.getImage();

    std::vector<Point2f> rays = {
        {0, 1}, {1, 0}, {1, 1} 
    };

    auto intersect = [] (const Point2f& head, const Point2f& end, 
                         const Point2f& origin, const Point2f& ray, 
                         std::vector<Point2f>& mp) {
        Point2f ad = end - head;
        Point2f bd = ray;
        float dx = origin.x - head.x;
        float dy = origin.y - head.y;
        float det = bd.x * ad.y - bd.y * ad.x;
        if (std::abs(det - 0.0000) < 1e-5)
            return 0;
        float u = (dy * bd.x - dx * bd.y) / det;
        float v = (dy * ad.x - dx * ad.y) / det;
        if (v < 0) return 0;
        if (u < 0 || u > 1.001)
            return 0;
        Point2f ret = ad * u + head;
        for (int i = 0; i < mp.size(); i++) 
            if (std::abs(ret.x - mp[i].x) < 1 && std::abs(ret.y - mp[i].y) < 1)
                return 0;
        
        mp.push_back(ret);
        return 1;
    };

    auto inPoly = [&] (Point2f p) {
        for (int i = 0; i < rays.size(); i++) {
            int ret = 0;
            std::vector<Point2f> mp;
            for (int j = 0; j < np; j++) 
                ret += intersect(ps[j], ps[(j + 1) % np], p, rays[i], mp);

            if (ret % 2 == 0)
                return false;
        }
        return true;
    };

    for (int i = 0; i < image.rows; i++) 
        for (int j = 0; j < image.cols; j++) 
            if (!inPoly(Point2f(i, j)))
                image.at<Vec4b>(i, j) = Vec4b(0, 0, 0, 0);
}

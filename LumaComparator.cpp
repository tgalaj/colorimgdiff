/*
MIT License

Copyright (c) 2020 Tomasz Ga³aj

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "LumaComparator.hpp"

LumaComparator::LumaComparator(const tinycolormap::ColormapType& colormap_type, const std::string& out_filename, unsigned width, unsigned height, int interpolation_ranges)
    : BaseComparator(colormap_type, out_filename, width, height, interpolation_ranges),
      m_mse         (0.0)
{
}

LumaComparator::~LumaComparator()
{
}

void LumaComparator::compare(const std::vector<uint8_t>& ref_img, const std::vector<uint8_t>& src_image)
{
    /* Calculate luminance of both images */
    auto ref_luma = luma(ref_img);
    auto src_luma = luma(src_image);

    /* Normalize both images */
    auto ref_norm_luma = normalize_image_linear(ref_luma, 0.0, 1.0);
    auto src_norm_luma = normalize_image_linear(src_luma, 0.0, 1.0);

    unsigned num_pixels = ref_luma.size();
    std::vector<double>  mse_image(num_pixels);

    m_mse = 0.0;

    for (unsigned i = 0; i < mse_image.size(); ++i)
    {
        /* Calculate MSE */
        double err = ref_norm_luma[i] - src_norm_luma[i];

        err    = err * err;
        m_mse += err;

        mse_image[i] = err;
    }

    m_mse /= num_pixels;

    auto mse_image_norm = normalize_image_linear(mse_image, 0.0, 1.0);
    save_diff_image(mse_image_norm);
}

double LumaComparator::get_error() const
{
    return m_mse;
}

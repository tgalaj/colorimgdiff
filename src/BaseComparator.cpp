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

#include "BaseComparator.hpp"

#include <algorithm>
#include <cmath>

#include <stb_image_write.h>

BaseComparator::BaseComparator(const tinycolormap::ColormapType& colormap_type, const std::string& out_filename, unsigned width, unsigned height, int interpolation_ranges)
    : m_colormap_type       (colormap_type),
      m_out_filename        (out_filename),
      m_width               (width),
      m_height              (height),
      m_interpolation_ranges(interpolation_ranges) {}

BaseComparator::~BaseComparator() {}

std::vector<uint8_t> BaseComparator::load_image(const std::string& filename, ImageMetadata& img_data)
{
    std::vector<uint8_t> img;
    int nr_channels_in_file;

    img_data.nr_channels = 3;
    auto* data = stbi_load(filename.c_str(), &img_data.width, &img_data.height, &nr_channels_in_file, img_data.nr_channels);

    if (data)
    {
        img = std::vector<uint8_t>(data, data + img_data.width * img_data.height * img_data.nr_channels);
        stbi_image_free(data);
    }

    return img;
}

std::vector<double> BaseComparator::luma(const std::vector<uint8_t>& img)
{
    const unsigned num_pixels = img.size() / 3;

    std::vector<double> luma(num_pixels);

    for (unsigned i = 0; i < num_pixels; ++i)
    {
        /* Conver to [0, 1] range */
        double r = img[3 * i + 0] / 255.0;
        double g = img[3 * i + 1] / 255.0;
        double b = img[3 * i + 2] / 255.0;

        /* Calculate luminance */
        luma[i] = r * 0.2126 + g * 0.7152 + b * 0.0722;
    }

    return luma;
}

std::vector<double> BaseComparator::normalize_image_linear(const std::vector<double>& img, double new_min, double new_max)
{
    const auto [min, max] = std::minmax_element(std::begin(img), std::end(img));

    double denom = (*max - *min);
    if (denom <= 0.0)
    {
        denom = 1.0;
    }

    double ratio = (new_max - new_min) / denom;

    std::vector<double> norm_img(img.size());

    for (unsigned int i = 0; i < img.size(); ++i)
    {
        norm_img[i] = (img[i] - *min) * ratio + new_min;
    }

    return norm_img;
}

std::vector<double> BaseComparator::rgb_2_lab(const std::vector<uint8_t>& img)
{
    std::vector<double> lab(img.size());
    std::array<double, 3> rgb;
    std::array<double, 3> xyz;

    double ref_x = 95.047;
    double ref_y = 100.000;
    double ref_z = 108.883;

    auto num_pixels = lab.size() / 3;

    for (unsigned i = 0; i < num_pixels; ++i)
    {
        /* Conver to [0, 1] range */
        rgb[0] = img[3 * i + 0] / 255.0;
        rgb[1] = img[3 * i + 1] / 255.0;
        rgb[2] = img[3 * i + 2] / 255.0;

        /* Convert to XYZ with respect to D65/2° standard illuminant. */
        for (auto& comp : rgb)
        {
            if (comp > 0.04045)
            {
                comp = std::pow((comp + 0.055) / 1.055, 2.4);
            }
            else
            {
                comp /= 12.92;
            }

            comp *= 100.0;
        }

        xyz[0] = rgb[0] * 0.4124 + rgb[1] * 0.3576 + rgb[2] * 0.1805;
        xyz[1] = rgb[0] * 0.2126 + rgb[1] * 0.7152 + rgb[2] * 0.0722;
        xyz[2] = rgb[0] * 0.0193 + rgb[1] * 0.1192 + rgb[2] * 0.9505;

        /* Convert to L*a*b* with respect to D65/2° standard illuminant. */
        xyz[0] /= ref_x;
        xyz[1] /= ref_y;
        xyz[2] /= ref_z;

        for (auto& comp : xyz)
        {
            if (comp > 0.008856)
            {
                comp = std::pow(comp, 1.0 / 3.0);
            }
            else
            {
                comp = (7.787 * comp) + (16.0 / 116.0);
            }
        }

        lab[3 * i + 0] = (116.0 * xyz[1]) - 16.0;
        lab[3 * i + 1] = 500.0 * (xyz[0] - xyz[1]);
        lab[3 * i + 2] = 200.0 * (xyz[1] - xyz[2]);
    }

    return lab;
}

void BaseComparator::save_diff_image(const std::vector<double>& error_img)
{
    std::vector<uint8_t> diff_image(error_img.size() * 3);

    for (unsigned i = 0; i < error_img.size(); ++i)
    {
        // Get Color from colormap
        tinycolormap::Color color(1.0, 1.0, 1.0);

        if (m_interpolation_ranges <= 0)
        {
            color = tinycolormap::GetColor(error_img[i], m_colormap_type);
        }
        else
        {
            // TODO
        }

        diff_image[3 * i + 0] = color.ri();
        diff_image[3 * i + 1] = color.gi();
        diff_image[3 * i + 2] = color.bi();
    }

    stbi_write_png(m_out_filename.c_str(), m_width, m_height, 3, diff_image.data(), 0);
}

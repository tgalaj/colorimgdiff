#pragma once

#include <string>
#include <vector>

#include <stb_image.h>
#include <tinycolormap.hpp>

struct ImageMetadata
{
	int width;
	int height;
	int nr_channels = 3;
};

class BaseComparator
{
public:
    BaseComparator(const tinycolormap::ColormapType& colormap_type, const std::string& out_filename, unsigned width, unsigned height, int interpolation_ranges);
    virtual ~BaseComparator();
    
    virtual void compare(const std::vector<uint8_t> & ref_img, const std::vector<uint8_t> & src_image) = 0;
    virtual double get_error() const = 0;

    static std::vector<uint8_t> load_image(const std::string& filename, ImageMetadata& img_data);
    static std::vector<double> luma(const std::vector<uint8_t>& img);

    /* Performs linear normalization: https://en.wikipedia.org/wiki/Normalization_(image_processing) */
    static std::vector<double> normalize_image_linear(const std::vector<double>& img, double new_min, double new_max);

    /* 
     * RGB -> XYZ -> L*a*b* conversion based on:
     * http://www.easyrgb.com/en/math.php 
     */
    static std::vector<double> rgb_2_lab(const std::vector<uint8_t>& img);

protected:
    virtual void save_diff_image(const std::vector<double> & error_img);

    std::string m_out_filename;
    tinycolormap::ColormapType m_colormap_type;
    unsigned m_width;
    unsigned m_height;
    int m_interpolation_ranges;
};
#include "LabComparator.hpp"

LabComparator::LabComparator(const tinycolormap::ColormapType& colormap_type, const std::string& out_filename, unsigned width, unsigned height, int interpolation_ranges)
    : BaseComparator(colormap_type, out_filename, width, height, interpolation_ranges),
      m_delta_e     (0.0)
{
}

LabComparator::~LabComparator()
{
}

void LabComparator::compare(const std::vector<uint8_t>& ref_img, const std::vector<uint8_t>& src_image)
{
    /* Convert RGB to L*a*b* */
    auto ref_lab = rgb_2_lab(ref_img);
    auto src_lab = rgb_2_lab(src_image);

    unsigned num_pixels = ref_lab.size() / 3;
    std::vector<double>  delta_e_image(num_pixels);

    std::array<double, 3> ref_lab_pixel;
    std::array<double, 3> src_lab_pixel;

    m_delta_e = 0.0;

    for (unsigned i = 0; i < delta_e_image.size(); ++i)
    {
        ref_lab_pixel = { ref_lab[3 * i + 0], ref_lab[3 * i + 1], ref_lab[3 * i + 2] };
        src_lab_pixel = { src_lab[3 * i + 0], src_lab[3 * i + 1], src_lab[3 * i + 2] };

        /* 
         * Calculate delta E based on https://sensing.konicaminolta.us/us/blog/identifying-color-differences-using-l-a-b-or-l-c-h-coordinates/ 
         */
        double err = std::sqrt((src_lab_pixel[0] - ref_lab_pixel[0]) * (src_lab_pixel[0] - ref_lab_pixel[0]) + 
                               (src_lab_pixel[1] - ref_lab_pixel[1]) * (src_lab_pixel[1] - ref_lab_pixel[1]) +
                               (src_lab_pixel[2] - ref_lab_pixel[2]) * (src_lab_pixel[2] - ref_lab_pixel[2]));
        m_delta_e += err;
        delta_e_image[i] = err;
    }

    m_delta_e /= num_pixels;

    auto delta_e_image_norm = normalize_image_linear(delta_e_image, 0.0, 1.0);
    save_diff_image(delta_e_image_norm);
}

double LabComparator::get_error() const
{
    return m_delta_e;
}

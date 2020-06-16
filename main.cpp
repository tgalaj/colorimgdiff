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

#include <algorithm>
#include <iostream>
#include <cmath>
#include <string>
#include <vector>

#include <cxxopts.hpp>
#include <stb_image.h>
#include <stb_image_write.h>
#include <tinycolormap.hpp>

struct ImageMetadata
{
    int width;
    int height;
    int nr_channels = 3;
};

tinycolormap::ColormapType setColormapType(const std::string& colormap_name)
{
    if (colormap_name == "Parula")
    {
        return tinycolormap::ColormapType::Parula;
    }
    else if (colormap_name == "Heat")
    {
        return tinycolormap::ColormapType::Heat;
    }
    else if (colormap_name == "Hot")
    {
        return tinycolormap::ColormapType::Hot;
    }
    else if (colormap_name == "Jet")
    {
        return tinycolormap::ColormapType::Jet;
    }
    else if (colormap_name == "Gray")
    {
        return tinycolormap::ColormapType::Gray;
    }
    else if (colormap_name == "Magma")
    {
        return tinycolormap::ColormapType::Magma;
    }
    else if (colormap_name == "Inferno")
    {
        return tinycolormap::ColormapType::Inferno;
    }
    else if (colormap_name == "Plasma")
    {
        return tinycolormap::ColormapType::Plasma;
    }
    else if (colormap_name == "Viridis")
    {
        return tinycolormap::ColormapType::Viridis;
    }
    else if (colormap_name == "Cividis")
    {
        return tinycolormap::ColormapType::Cividis;
    }
    else if (colormap_name == "Github")
    {
        return tinycolormap::ColormapType::Github;
    }

    return tinycolormap::ColormapType::Hot;
}

std::vector<uint8_t> load_image(const std::string & filename, ImageMetadata& img_data)
{
    int nr_channels_in_file;

    img_data.nr_channels = 3;
    auto* data = stbi_load(filename.c_str(), &img_data.width, &img_data.height, &nr_channels_in_file, img_data.nr_channels);

    std::vector<uint8_t> img(data, data + img_data.width * img_data.height * img_data.nr_channels);

    stbi_image_free(data);

    return img;
}

std::vector<double> luma(const std::vector<uint8_t>& img)
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

/* Performs linear normalization: https://en.wikipedia.org/wiki/Normalization_(image_processing) */
std::vector<double> normalize_image_linear(const std::vector<double> & img, double new_min, double new_max)
{
    const auto [min, max] = std::minmax_element(std::begin(img), std::end(img));
    const double ratio = (new_max - new_min) / (*max - *min);
    
    std::vector<double> norm_img(img.size());

    for (unsigned int i = 0; i < img.size(); ++i)
    {
        norm_img[i] = (img[i] - *min) * ratio + new_min;
    }

    return norm_img;
}

int main(int argc, char* argv[])
{
    cxxopts::Options options("colorimgdiff", "Creates diff image of ref(erence) and src (source) images. It simply computes luma difference between ref and src.\n");
    options.add_options()("r,ref", "Relative path to reference image WITH extension [REQUIRED]",                 cxxopts::value<std::string>())
                         ("s,src", "Relative path to source image WITH extension    [REQUIRED]",                 cxxopts::value<std::string>())
                         ("o,out", "Relative path to output image WITHOUT extension (it'll be a PNG image)",     cxxopts::value<std::string>()->default_value("output_diff"))
                         ("c,colormap", "Changes the default colormap. Possible options are: Parula, Heat, "
                                        "Hot, Jet, Gray, Magma, Inferno, Plasma, Viridis, Cividis, Github.",     cxxopts::value<std::string>()->default_value("Hot"))
                         //("i,interpolate", "Choose a value from range [1, 255] if you want to disable color "
                         //                  "interpolation (default) and want to assign several values to the "
                         //                  "same color.",                                                        cxxopts::value<int>()->default_value("-1"))
                         ("v,verbose", "Verbose output",                                                         cxxopts::value<bool>()->default_value("false"))
                         ("h,help", "Prints this message");

    auto cmd_result = options.parse(argc, argv);

    if (cmd_result.count("help"))
    {
        std::cout << options.help() << std::endl;
        exit(0);
    }

    if (!cmd_result.count("ref") || !cmd_result.count("src"))
    {
        std::cout << options.help() << std::endl;
        exit(0);
    }

    int interpolation_ranges = -1;// cmd_result["interpolate"].as<int>();
    bool verbose_output      = cmd_result["verbose"].as<bool>();
    auto colormap_type       = setColormapType(cmd_result["colormap"].as<std::string>());
   

    std::string ref_filename = cmd_result["ref"].as<std::string>();
    std::string src_filename = cmd_result["src"].as<std::string>();
    std::string out_filename = cmd_result["out"].as<std::string>() + ".png";

    ImageMetadata ref_metadata, src_metadata;

    auto ref_data = load_image(ref_filename, ref_metadata);
    auto src_data = load_image(src_filename, src_metadata);

    if (verbose_output)
    {
        if (ref_data.empty())
        {
            std::cerr << "Couldn't load " << ref_filename << std::endl;
            return 1;
        }

        if (src_data.empty())
        {
            std::cerr << "Couldn't load " << src_filename << std::endl;
            return 1;
        }

        if (ref_data.size() != src_data.size())
        {
            std::cerr << "Ref ans Src images' dimensions don't match!" << std::endl;
            return 1;
        }
    }

    /* Calculate luminance of both images */
    auto ref_luma = luma(ref_data);
    auto src_luma = luma(src_data);

    /* Normalize both images */
    auto ref_norm_luma = normalize_image_linear(ref_luma, 0.0, 1.0);
    auto src_norm_luma = normalize_image_linear(src_luma, 0.0, 1.0);

    double mse = 0.0;

    std::vector<uint8_t> diff_image(ref_norm_luma.size() * 3);
    for (unsigned i = 0; i < ref_norm_luma.size(); ++i)
    {
        // Calculate difference
        double err = ref_norm_luma[i] - src_norm_luma[i];
        mse += err * err;

        // Get Color from colormap
        tinycolormap::Color color(1.0, 1.0, 1.0);
        
        if (interpolation_ranges < 0)
        {
            color = tinycolormap::GetColor(std::fabs(err), colormap_type);
        }
        else
        {
            // TODO
        }

        diff_image[3 * i + 0] = color.ri();
        diff_image[3 * i + 1] = color.gi();
        diff_image[3 * i + 2] = color.bi();
    }

    stbi_write_png(out_filename.c_str(), ref_metadata.width, ref_metadata.height, 3, diff_image.data(), 0);

    mse = mse / ref_norm_luma.size();

    if (verbose_output)
    {
        std::cout << "Saved image " << out_filename   << std::endl;
        std::cout << "MSE:  "       << mse            << std::endl;
        std::cout << "RMSE: "       << std::sqrt(mse) << std::endl;
    }

    return 0;
}
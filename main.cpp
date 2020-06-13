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
    int nr_channels;
};

uint8_t* load_image(const std::string & filename, ImageMetadata& img_data)
{
    return stbi_load(filename.c_str(), &img_data.width, &img_data.height, &img_data.nr_channels, 3);
}

int main(int argc, char* argv[])
{
    // TODO:
    // - Add cmd arg option to choose a desired colormap

    cxxopts::Options options("colorimgdiff", "Creates diff image of ref(erence) and src (source) images. It simply computes luma difference between ref and src.\n");
    options.add_options()("r,ref", "Relative path to reference image WITH extension [REQUIRED]",             cxxopts::value<std::string>())
                         ("s,src", "Relative path to source image WITH extension    [REQUIRED]",             cxxopts::value<std::string>())
                         ("o,out", "Relative path to output image WITHOUT extension (it'll be a PNG image)", cxxopts::value<std::string>()->default_value("output_diff"))
                         ("v,verbose", "Verbose output",                                                     cxxopts::value<bool>()->default_value("false"))
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

    bool verbose_output = cmd_result["verbose"].as<bool>();

    std::string ref_filename = cmd_result["ref"].as<std::string>();
    std::string src_filename = cmd_result["src"].as<std::string>();
    std::string out_filename = cmd_result["out"].as<std::string>() + ".png";

    ImageMetadata ref_metadata, src_metadata;

    auto ref_data = load_image(ref_filename, ref_metadata);
    auto src_data = load_image(src_filename, src_metadata);

    if (verbose_output)
    {
        if (!ref_data)
        {
            std::cerr << "Couldn't load " << ref_filename << std::endl;
            return 1;
        }

        if (!src_data)
        {
            std::cerr << "Couldn't load " << src_filename << std::endl;
            return 1;
        }

        if (ref_metadata.width != src_metadata.width || ref_metadata.height != src_metadata.height)
        {
            std::cerr << "Ref ans Src images' dimensions don't match!" << std::endl;
            return 1;
        }
    }

    std::vector<uint8_t> diff_image(ref_metadata.width * ref_metadata.height * 3);
    for (unsigned i = 0; i < ref_metadata.width * ref_metadata.height; ++i)
    {
        double ref_r = ref_data[3 * i + 0] / 255.0;
        double ref_g = ref_data[3 * i + 1] / 255.0;
        double ref_b = ref_data[3 * i + 2] / 255.0;

        double src_r = src_data[3 * i + 0] / 255.0;
        double src_g = src_data[3 * i + 1] / 255.0;
        double src_b = src_data[3 * i + 2] / 255.0;

        // Calculate luminance
        double ref_luma = ref_r * 0.2126 + ref_g * 0.7152 + ref_b * 0.0722;
        double src_luma = src_r * 0.2126 + src_g * 0.7152 + src_b * 0.0722;;

        // Calculate difference
        double err = std::fabs(ref_luma - src_luma);

        // Get Color from colormap
        auto color = tinycolormap::GetColor(err, tinycolormap::ColormapType::Hot);
        diff_image[3 * i + 0] = color.ri();
        diff_image[3 * i + 1] = color.gi();
        diff_image[3 * i + 2] = color.bi();
    }

    stbi_image_free(ref_data);
    stbi_image_free(src_data);

    stbi_write_png(out_filename.c_str(), ref_metadata.width, ref_metadata.height, 3, diff_image.data(), 0);

    if (verbose_output)
    {
        std::cout << "Saved image " << out_filename << std::endl;
    }

    return 0;
}
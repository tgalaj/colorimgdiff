/*
MIT License

Copyright (c) 2020 Tomasz Gałaj

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

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <cxxopts.hpp>

#include "BaseComparator.hpp"
#include "LumaComparator.hpp"
#include "LabComparator.hpp"

tinycolormap::ColormapType setColormapType(const std::string& colormap_name)
{
    static std::unordered_map<std::string, tinycolormap::ColormapType> colormaps = 
    {
        {"Parula",  tinycolormap::ColormapType::Parula},
        {"Heat",    tinycolormap::ColormapType::Heat},
        {"Hot",     tinycolormap::ColormapType::Hot},
        {"Jet",     tinycolormap::ColormapType::Jet},
        {"Gray",    tinycolormap::ColormapType::Gray},
        {"Magma",   tinycolormap::ColormapType::Magma},
        {"Inferno", tinycolormap::ColormapType::Inferno},
        {"Plasma",  tinycolormap::ColormapType::Plasma},
        {"Viridis", tinycolormap::ColormapType::Viridis},
        {"Cividis", tinycolormap::ColormapType::Cividis},
        {"Github",  tinycolormap::ColormapType::Github}
    };

    if (colormaps.count(colormap_name))
    {
        return colormaps[colormap_name];
    }

    return colormaps["Hot"];
}

int main(int argc, char* argv[])
{
    cxxopts::Options options("colorimgdiff", "Creates diff image of ref(erence) and src (source) images.\n");
    options.add_options()("r,ref",      "Relative path to reference image WITH extension [REQUIRED]",             cxxopts::value<std::string>())
                         ("s,src",      "Relative path to source image WITH extension    [REQUIRED]",             cxxopts::value<std::string>())
                         ("o,out",      "Relative path to output image WITHOUT extension (it'll be a PNG image)", cxxopts::value<std::string>()->default_value("output_diff"))
                         ("c,colormap", "Changes the default colormap. Possible options are: Parula, Heat, "
                                        "Hot, Jet, Gray, Magma, Inferno, Plasma, Viridis, Cividis, Github.",      cxxopts::value<std::string>()->default_value("Hot"))
                         //("i,interpolate", "Choose a value from range [1, 255] if you want to disable color "
                         //                  "interpolation (default) and want to assign several values to the "
                         //                  "same color.",                                                       cxxopts::value<int>()->default_value("-1"))
                         ("m,mode", "Sets the comparison mode. Available options are: Luma, Lab.",                cxxopts::value<std::string>()->default_value("Luma"))
                         ("v,verbose",  "Verbose output",                                                         cxxopts::value<bool>()->default_value("false"))
                         ("h,help",     "Prints this message");
    
    options.positional_help("<ref_image> <src_image>");
    options.parse_positional({ "ref", "src" });

    auto cmd_result = options.parse(argc, argv);

    if (cmd_result.count("help"))
    {
        std::cout << options.help() << std::endl;
        exit(0);
    }

    if (!cmd_result.count("ref") || !cmd_result.count("src"))
    {
        std::cerr << "ERROR: You have to specify relative paths to reference and source images repectively!\n\n";
        std::cout << options.help() << std::endl;
        exit(0);
    }

    int interpolation_ranges = -1;// cmd_result["interpolate"].as<int>();
    bool verbose_output      = cmd_result["verbose"].as<bool>();
    auto comp_mode           = cmd_result["mode"].as<std::string>();
    auto colormap_type       = setColormapType(cmd_result["colormap"].as<std::string>());

    std::string ref_filename = cmd_result["ref"].as<std::string>();
    std::string src_filename = cmd_result["src"].as<std::string>();
    std::string out_filename = cmd_result["out"].as<std::string>() + ".png";

    ImageMetadata ref_metadata, src_metadata;

    auto ref_data = BaseComparator::load_image(ref_filename, ref_metadata);
    auto src_data = BaseComparator::load_image(src_filename, src_metadata);

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

    std::shared_ptr<BaseComparator> comparator;

    if (comp_mode == "Luma")
    {
        if (verbose_output)
        {
            std::cout << "Comparing luminance..." << std::endl;
        }

        comparator = std::make_shared<LumaComparator>(colormap_type, out_filename, ref_metadata.width, ref_metadata.height, interpolation_ranges);
        comparator->compare(ref_data, src_data);

        if (verbose_output)
        {
            std::cout << "Saved image " << out_filename << std::endl;
            std::cout << "MSE:  " << comparator->get_error() << std::endl;
            std::cout << "RMSE: " << std::sqrt(comparator->get_error()) << std::endl;
        }
    }

    if (comp_mode == "Lab")
    {
        if (verbose_output)
        {
            std::cout << "Comparing color in L*a*b* space..." << std::endl;
        }
        
        comparator = std::make_shared<LabComparator>(colormap_type, out_filename, ref_metadata.width, ref_metadata.height, interpolation_ranges);
        comparator->compare(ref_data, src_data);

        if (verbose_output)
        {
            std::cout << "Saved image " << out_filename << std::endl;
            std::cout << "delta E*ab:  " << comparator->get_error() << std::endl;
        }
    }

    return 0;
}
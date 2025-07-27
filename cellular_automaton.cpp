#include <iostream>
#include <vector>
#include <random>
#include <tuple>
#include <map>
#include <string>
#include <cxxopts.hpp>
#include <lodepng.h>

int main(int argc, char* argv[]) {
    // Parse command line arguments
    cxxopts::Options options("CellularAutomaton", "Generate an elementary cellular automaton");
    options.add_options()
        ("x,width", "Width", cxxopts::value<int>()->default_value("322"))
        ("y,height", "Height", cxxopts::value<int>()->default_value("322"))
        ("r,rulenumber", "Rule number", cxxopts::value<int>()->default_value("18"))
        ("o,outputfile", "Output file", cxxopts::value<std::string>()->default_value("out.png"))
        ("s,scalefactor", "Scale the output image by an integer amount", cxxopts::value<int>()->default_value("1"));

    auto args = options.parse(argc, argv);

    int width = args["width"].as<int>();
    int height = args["height"].as<int>();
    int rulenumber = args["rulenumber"].as<int>();
    int scalefactor = args["scalefactor"].as<int>();
    std::string outputfile = args["outputfile"].as<std::string>();

    // Define colors of the output image
    const std::vector<unsigned char> true_pixel = {255, 255, 255, 255};
    const std::vector<unsigned char> false_pixel = {0, 0, 0, 255};

    // Random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 1);

    // Generate rule map
    std::map<std::tuple<bool, bool, bool>, bool> rule;
    int temp_rulenumber = rulenumber;
    for (bool left : {false, true}) {
        for (bool middle : {false, true}) {
            for (bool right : {false, true}) {
                rule[{left, middle, right}] = temp_rulenumber % 2 == 1;
                temp_rulenumber /= 2;
            }
        }
    }

    // Generate cellular automaton
    std::vector<std::vector<bool>> ca(height, std::vector<bool>(width));
    
    // Initialize first row randomly
    for (int x = 0; x < width; ++x) {
        ca[0][x] = dis(gen);
    }

    // Generate succeeding generations
    for (int y = 1; y < height; ++y) {
        ca[y][0] = dis(gen); // Left edge
        for (int x = 1; x < width - 1; ++x) {
            ca[y][x] = rule[{ca[y-1][x-1], ca[y-1][x], ca[y-1][x+1]}];
        }
        ca[y][width-1] = dis(gen); // Right edge
    }

    // Create image buffer
    std::vector<unsigned char> image(width * height * 4);
    
    std::cout << "Placing pixels..." << std::endl;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = 4 * (y * width + x);
            const auto& pixel = ca[y/scalefactor][x/scalefactor] ? true_pixel : false_pixel;
            image[index] = pixel[0];
            image[index + 1] = pixel[1];
            image[index + 2] = pixel[2];
            image[index + 3] = pixel[3];
        }
    }

    // Save image
    std::cout << "Saving image..." << std::endl;
    unsigned error = lodepng::encode(outputfile, image, width, height);
    if (error) {
        std::cerr << "PNG encoding error " << error << ": " << lodepng_error_text(error) << std::endl;
        return 1;
    }

    std::cout << "Done!" << std::endl;
    return 0;
}
/*
main.cpp
CSPB 1300 Image Processing Application

PLEASE FILL OUT THIS SECTION PRIOR TO SUBMISSION

- Your name:
    Nathan J Harris

- All project requirements fully met? (YES or NO):
    <ANSWER>

- If no, please explain what you could not get to work:
    <ANSWER>

- Did you do any optional enhancements? If so, please explain:
    <ANSWER>
*/

#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
using namespace std;

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION BELOW                                    //
//***************************************************************************************************//

// Pixel structure
struct Pixel
{
    // Red, green, blue color values
    int red;
    int green;
    int blue;
};

/**
 * Gets an integer from a binary stream.
 * Helper function for read_image()
 * @param stream the stream
 * @param offset the offset at which to read the integer
 * @param bytes  the number of bytes to read
 * @return the integer starting at the given offset
 */
int get_int(fstream &stream, int offset, int bytes)
{
    stream.seekg(offset);
    int result = 0;
    int base = 1;
    for (int i = 0; i < bytes; i++)
    {
        result = result + stream.get() * base;
        base = base * 256;
    }
    return result;
}

/**
 * Reads the BMP image specified and returns the resulting image as a vector
 * @param filename BMP image filename
 * @return the image as a vector of vector of Pixels
 */
vector<vector<Pixel>> read_image(string filename)
{
    // Open the binary file
    fstream stream;
    stream.open(filename, ios::in | ios::binary);

    // Get the image properties
    int file_size = get_int(stream, 2, 4);
    int start = get_int(stream, 10, 4);
    int width = get_int(stream, 18, 4);
    int height = get_int(stream, 22, 4);
    int bits_per_pixel = get_int(stream, 28, 2);

    // Scan lines must occupy multiples of four bytes
    int scanline_size = width * (bits_per_pixel / 8);
    int padding = 0;
    if (scanline_size % 4 != 0)
    {
        padding = 4 - scanline_size % 4;
    }

    // Return empty vector if this is not a valid image
    if (file_size != start + (scanline_size + padding) * height)
    {
        return {};
    }

    // Create a vector the size of the input image
    vector<vector<Pixel>> image(height, vector<Pixel>(width));

    int pos = start;
    // For each row, starting from the last row to the first
    // Note: BMP files store pixels from bottom to top
    for (int i = height - 1; i >= 0; i--)
    {
        // For each column
        for (int j = 0; j < width; j++)
        {
            // Go to the pixel position
            stream.seekg(pos);

            // Save the pixel values to the image vector
            // Note: BMP files store pixels in blue, green, red order
            image[i][j].blue = stream.get();
            image[i][j].green = stream.get();
            image[i][j].red = stream.get();

            // We are ignoring the alpha channel if there is one

            // Advance the position to the next pixel
            pos = pos + (bits_per_pixel / 8);
        }

        // Skip the padding at the end of each row
        stream.seekg(padding, ios::cur);
        pos = pos + padding;
    }

    // Close the stream and return the image vector
    stream.close();
    return image;
}

/**
 * Sets a value to the char array starting at the offset using the size
 * specified by the bytes.
 * This is a helper function for write_image()
 * @param arr    Array to set values for
 * @param offset Starting index offset
 * @param bytes  Number of bytes to set
 * @param value  Value to set
 * @return nothing
 */
void set_bytes(unsigned char arr[], int offset, int bytes, int value)
{
    for (int i = 0; i < bytes; i++)
    {
        arr[offset + i] = (unsigned char)(value >> (i * 8));
    }
}

/**
 * Write the input image to a BMP file name specified
 * @param filename The BMP file name to save the image to
 * @param image    The input image to save
 * @return True if successful and false otherwise
 */
bool write_image(string filename, const vector<vector<Pixel>> &image)
{
    // Get the image width and height in pixels
    int width_pixels = image[0].size();
    int height_pixels = image.size();

    // Calculate the width in bytes incorporating padding (4 byte alignment)
    int width_bytes = width_pixels * 3;
    int padding_bytes = 0;
    padding_bytes = (4 - width_bytes % 4) % 4;
    width_bytes = width_bytes + padding_bytes;

    // Pixel array size in bytes, including padding
    int array_bytes = width_bytes * height_pixels;

    // Open a file stream for writing to a binary file
    fstream stream;
    stream.open(filename, ios::out | ios::binary);

    // If there was a problem opening the file, return false
    if (!stream.is_open())
    {
        return false;
    }

    // Create the BMP and DIB Headers
    const int BMP_HEADER_SIZE = 14;
    const int DIB_HEADER_SIZE = 40;
    unsigned char bmp_header[BMP_HEADER_SIZE] = {0};
    unsigned char dib_header[DIB_HEADER_SIZE] = {0};

    // BMP Header
    set_bytes(bmp_header, 0, 1, 'B');                                             // ID field
    set_bytes(bmp_header, 1, 1, 'M');                                             // ID field
    set_bytes(bmp_header, 2, 4, BMP_HEADER_SIZE + DIB_HEADER_SIZE + array_bytes); // Size of BMP file
    set_bytes(bmp_header, 6, 2, 0);                                               // Reserved
    set_bytes(bmp_header, 8, 2, 0);                                               // Reserved
    set_bytes(bmp_header, 10, 4, BMP_HEADER_SIZE + DIB_HEADER_SIZE);              // Pixel array offset

    // DIB Header
    set_bytes(dib_header, 0, 4, DIB_HEADER_SIZE); // DIB header size
    set_bytes(dib_header, 4, 4, width_pixels);    // Width of bitmap in pixels
    set_bytes(dib_header, 8, 4, height_pixels);   // Height of bitmap in pixels
    set_bytes(dib_header, 12, 2, 1);              // Number of color planes
    set_bytes(dib_header, 14, 2, 24);             // Number of bits per pixel
    set_bytes(dib_header, 16, 4, 0);              // Compression method (0=BI_RGB)
    set_bytes(dib_header, 20, 4, array_bytes);    // Size of raw bitmap data (including padding)
    set_bytes(dib_header, 24, 4, 2835);           // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 28, 4, 2835);           // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 32, 4, 0);              // Number of colors in palette
    set_bytes(dib_header, 36, 4, 0);              // Number of important colors

    // Write the BMP and DIB Headers to the file
    stream.write((char *)bmp_header, sizeof(bmp_header));
    stream.write((char *)dib_header, sizeof(dib_header));

    // Initialize pixel and padding
    unsigned char pixel[3] = {0};
    unsigned char padding[3] = {0};

    // Pixel Array (Left to right, bottom to top, with padding)
    for (int h = height_pixels - 1; h >= 0; h--)
    {
        for (int w = 0; w < width_pixels; w++)
        {
            // Write the pixel (Blue, Green, Red)
            pixel[0] = image[h][w].blue;
            pixel[1] = image[h][w].green;
            pixel[2] = image[h][w].red;
            stream.write((char *)pixel, 3);
        }
        // Write the padding bytes
        stream.write((char *)padding, padding_bytes);
    }

    // Close the stream and return true
    stream.close();
    return true;
}

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION ABOVE                                    //
//***************************************************************************************************//

// g++ -std=c++11 -o main main.cpp
// ./main

// g++ -std=c++11 -o main main.cpp && ./main

//
// YOUR FUNCTION DEFINITIONS HER
//

// int main()
// {

//     //
//     // YOUR CODE HERE
//     //
//     cout << "\n\n\nThis line should be your own code!\n\n\n";

//     return 0;
// }

vector<vector<Pixel>> process_1(const vector<vector<Pixel>> &image)
{
    // Get the number of rows/columns from the input 2D vector (remember: num_rows is height, num_columns is width)
    int height = image.size();
    int width = image[0].size();

    // Define a new 2D vector the same size as the input 2D vector

    vector<vector<Pixel>> new_img(height, vector<Pixel>(width));

    // For each of the rows in the input 2D vector
    // For each of the columns in the input 2D vector
    // Get the color values for the pixel located at this row and column in the input 2D vector

    Pixel this_pixel;
    int red;
    int green;
    int blue;

    double distance;
    double scaling_factor;

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            distance = sqrt(pow(col - width / 2, 2) + pow(row - height / 2, 2));
            scaling_factor = (height - distance) / height;
            this_pixel = image[row][col];
            red = this_pixel.red * scaling_factor;
            green = this_pixel.green * scaling_factor;
            blue = this_pixel.blue * scaling_factor;

            new_img[row][col].red = red;
            new_img[row][col].green = green;
            new_img[row][col].blue = blue;
        }
    }
    return new_img;

    // Perform the operation on the color values (refer to Runestone for this)

    // Save the new color values to the corresponding pixel located at this row and column in the new 2D vector

    // Return the new 2D vector after the nested for loop is complete
}

vector<vector<Pixel>> process_2(const vector<vector<Pixel>> &image)
{
    // Get the number of rows/columns from the input 2D vector (remember: num_rows is height, num_columns is width)
    int height = image.size();
    int width = image[0].size();

    // Define a new 2D vector the same size as the input 2D vector

    vector<vector<Pixel>> new_img(height, vector<Pixel>(width));

    // For each of the rows in the input 2D vector
    // For each of the columns in the input 2D vector
    // Get the color values for the pixel located at this row and column in the input 2D vector

    Pixel this_pixel;
    int red;
    int green;
    int blue;

    double avg_val;
    double scaling_factor = 0.3;

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            this_pixel = image[row][col];
            red = this_pixel.red;
            green = this_pixel.green;
            blue = this_pixel.blue;
            avg_val = (red + green + blue) / 3;

            if (avg_val > 169)
            {
                red = (255 - (255 - red) * scaling_factor);
                green = (255 - (255 - green) * scaling_factor);
                blue = (255 - (255 - blue) * scaling_factor);
            }
            if (avg_val < 90)
            {
                red = red * scaling_factor;
                green = green * scaling_factor;
                blue = blue * scaling_factor;
            }
            // if (avg_val > 89 && avg_val < 170)
            // {
            // }

            new_img[row][col].red = red;
            new_img[row][col].green = green;
            new_img[row][col].blue = blue;
        }
    }
    return new_img;

    // Perform the operation on the color values (refer to Runestone for this)

    // Save the new color values to the corresponding pixel located at this row and column in the new 2D vector

    // Return the new 2D vector after the nested for loop is complete
}

vector<vector<Pixel>> process_3(const vector<vector<Pixel>> &image)
{
    // Get the number of rows/columns from the input 2D vector (remember: num_rows is height, num_columns is width)
    int height = image.size();
    int width = image[0].size();

    // Define a new 2D vector the same size as the input 2D vector

    vector<vector<Pixel>> new_img(height, vector<Pixel>(width));

    // For each of the rows in the input 2D vector
    // For each of the columns in the input 2D vector
    // Get the color values for the pixel located at this row and column in the input 2D vector

    Pixel this_pixel;
    int red;
    int green;
    int blue;
    int gray_val;

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            this_pixel = image[row][col];
            red = this_pixel.red;
            green = this_pixel.green;
            blue = this_pixel.blue;

            gray_val = (red + green + blue) / 3;

            new_img[row][col].red = gray_val;
            new_img[row][col].green = gray_val;
            new_img[row][col].blue = gray_val;
        }
    }
    return new_img;

    // Perform the operation on the color values (refer to Runestone for this)

    // Save the new color values to the corresponding pixel located at this row and column in the new 2D vector

    // Return the new 2D vector after the nested for loop is complete
}

int main()
{
    // Read in BMP image file into a 2D vector (using read_image function)

    vector<vector<Pixel>> image = read_image("sample.bmp");

    // Call process_1 function using the input 2D vector and save the result returned to a new 2D vector

    vector<vector<Pixel>> new_image = process_3(image);

    // Write the resulting 2D vector to a new BMP image file (using write_image function)

    write_image("new_sample.bmp", new_image);

    return 0;
}
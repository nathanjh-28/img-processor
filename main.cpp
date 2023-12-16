/*
main.cpp
CSPB 1300 Image Processing Application

PLEASE FILL OUT THIS SECTION PRIOR TO SUBMISSION

- Your name:
    Nathan J Harris

- All project requirements fully met? (YES or NO):
    YES

- If no, please explain what you could not get to work:
    N/A

- Did you do any optional enhancements? If so, please explain:
    Mirror image Horizontally
    Mirror image Vertically
    Blend 2 images by averaging pixels
    weighted Blend of 2 images
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

// --------------------------------------------------------------------------------------------------//

//***************************************************************************************************//
// PROCESSES 1 - 10
//***************************************************************************************************//

// ________________________________________________________ PROCESS 1 Vignette

/**
 * Description: Adds vignette effect to image (dark corners)
 * @param 2d vector of type Pixel
 * @return a new 2d vector of type pixel modified
 */

vector<vector<Pixel>> process_1(const vector<vector<Pixel>> &image)
{
    int height = image.size();
    int width = image[0].size();

    vector<vector<Pixel>> new_img(height, vector<Pixel>(width));

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
}
// ________________________________________________________ PROCESS 2 Clarendon

/**
 * Description: Adds Clarendon effect to image (darks darker and lights lighter) by a scaling factor
 * @param 2d vector of type Pixel
 * @param floating point scaling factor number
 * @return a new 2d vector of type pixel modified
 */

vector<vector<Pixel>> process_2(const vector<vector<Pixel>> &image, double scaling_factor)
{
    int height = image.size();
    int width = image[0].size();

    vector<vector<Pixel>> new_img(height, vector<Pixel>(width));

    Pixel this_pixel;
    int red;
    int green;
    int blue;

    double avg_val;

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

            new_img[row][col].red = red;
            new_img[row][col].green = green;
            new_img[row][col].blue = blue;
        }
    }
    return new_img;
}
// ________________________________________________________ PROCESS 3 Grayscale

/**
 * Description: Grayscale image.
 * @param 2d vector of type Pixel
 * @return a new 2d vector of type pixel modified
 */

vector<vector<Pixel>> process_3(const vector<vector<Pixel>> &image)
{
    int height = image.size();
    int width = image[0].size();

    vector<vector<Pixel>> new_img(height, vector<Pixel>(width));

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
}
// ________________________________________________________ PROCESS 4 Rotate 90 degrees

/**
 * Description: Rotate image 90 degrees clockwise
 * @param 2d vector of type Pixel
 * @return a new 2d vector of type pixel modified
 */

vector<vector<Pixel>> process_4(const vector<vector<Pixel>> &image)
{
    int height = image.size();
    int width = image[0].size();

    vector<vector<Pixel>> new_img(width, vector<Pixel>(height));

    Pixel this_pixel;

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            this_pixel = image[row][col];
            new_img[col][(height - 1) - row] = this_pixel;
        }
    }
    return new_img;
}

// ________________________________________________________ PROCESS 5 Rotate multiple 90 degrees

/**
 * Description: Rotate image by 90 degrees.
 * @param 2d vector of type Pixel
 * @return a new 2d vector of type pixel modified
 */

vector<vector<Pixel>> rotate_by_90(const vector<vector<Pixel>> &image)
{
    int height = image.size();
    int width = image[0].size();

    vector<vector<Pixel>> new_img(width, vector<Pixel>(height));

    Pixel this_pixel;

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            this_pixel = image[row][col];
            new_img[col][(height - 1) - row] = this_pixel;
        }
    }
    return new_img;
}

/**
 * Description: Rotates image by a specified number of multiples of 90 degrees clockwise
 * @param 2d vector of type Pixel
 * @param integer multiple
 * @return a new 2d vector of type pixel modified
 */

vector<vector<Pixel>> process_5(const vector<vector<Pixel>> &image, int num)
{

    int angle = num * 90;

    if (angle % 360 == 0)
    {
        return image;
    }

    if (angle % 360 == 90)
    {
        // rotate by 90
        return rotate_by_90(image);
    }
    if (angle % 360 == 180)
    {
        // rotate by 180
        return rotate_by_90(rotate_by_90(image));
    }
    else
    {
        return rotate_by_90(rotate_by_90(rotate_by_90(image)));
    }
}

// ________________________________________________________ PROCESS 6 Enlarge

/**
 * Description: Enlarges the image in the x and y direction
 * @param 2d vector of type Pixel
 * @param integer number to scale width (x)
 * @param integer number to scale height (y)
 * @return a new 2d vector of type pixel modified
 */

vector<vector<Pixel>> process_6(const vector<vector<Pixel>> &image, int x_scale, int y_scale)
{
    int height = image.size();
    int new_height = height * y_scale;
    int width = image[0].size();
    int new_width = width * x_scale;
    Pixel this_pixel;
    int reduced_col;
    int reduced_row;
    vector<vector<Pixel>> new_img(new_height, vector<Pixel>(new_width));
    for (int row = 0; row < new_height; row++)
    {
        for (int col = 0; col < new_width * x_scale; col++)
        {
            reduced_col = col / x_scale;
            reduced_row = row / y_scale;
            this_pixel = image[reduced_row][reduced_col];
            new_img[row][col] = this_pixel;
        }
    }
    return new_img;
}

// ________________________________________________________ PROCESS 7 High contrast

/**
 * Description: Convert image to high contrast (black and white only)
 * @param 2d vector of type Pixel
 * @return a new 2d vector of type pixel modified
 */

vector<vector<Pixel>> process_7(const vector<vector<Pixel>> &image)
{
    int height = image.size();
    int width = image[0].size();

    vector<vector<Pixel>> new_img(height, vector<Pixel>(width));

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

            if (gray_val < 128)
            {
                new_img[row][col].red = 0;
                new_img[row][col].green = 0;
                new_img[row][col].blue = 0;
            }
            if (gray_val > 127)
            {
                new_img[row][col].red = 255;
                new_img[row][col].green = 255;
                new_img[row][col].blue = 255;
            }
        }
    }
    return new_img;
}
// ________________________________________________________ PROCESS 8 Lighten

/**
 * Description: Lightens image by a scaling factor
 * @param 2d vector of type Pixel
 * @param floating point scaling factor for lightening image
 * @return a new 2d vector of type pixel modified
 */

vector<vector<Pixel>> process_8(const vector<vector<Pixel>> &image, double scaling_factor)
{
    int height = image.size();
    int width = image[0].size();

    vector<vector<Pixel>> new_img(height, vector<Pixel>(width));

    Pixel this_pixel;
    int red;
    int green;
    int blue;

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            this_pixel = image[row][col];
            red = 255 - (255 - this_pixel.red) * scaling_factor;
            green = 255 - (255 - this_pixel.green) * scaling_factor;
            blue = 255 - (255 - this_pixel.blue) * scaling_factor;

            new_img[row][col].red = red;
            new_img[row][col].green = green;
            new_img[row][col].blue = blue;
        }
    }
    return new_img;
}

// ________________________________________________________ PROCESS 9 Darken

/**
 * Description: Darkens image by a scaling factor
 * @param 2d vector of type Pixel
 * @param floating point scaling factor for darkening image
 * @return a new 2d vector of type pixel modified
 */

vector<vector<Pixel>> process_9(const vector<vector<Pixel>> &image, double scaling_factor)
{
    int height = image.size();
    int width = image[0].size();

    vector<vector<Pixel>> new_img(height, vector<Pixel>(width));

    Pixel this_pixel;
    int red;
    int green;
    int blue;

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
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
}

// ________________________________________________________ PROCESS 10 Black, white, red, green, blue

/**
 * Description: Selects the max value from a pixel, red, green or blue.
 * @param int red
 * @param int green
 * @param int blue
 * @return max value of the three rgb values
 */

int rgb_max(int red, int green, int blue)
{
    if (red >= green && red >= blue)
    {
        return red;
    }
    else if (green >= red && green >= blue)
    {
        return green;
    }
    else if (blue >= red && blue >= green)
    {
        return blue;
    }
    else if (red == green && green == blue)
    {
        return red;
    }
    return 0;
}

/**
 * Description: Converts image to only black, white, red, blue, and green
 * @param 2d vector of type Pixel
 * @return a new 2d vector of type pixel modified
 */

vector<vector<Pixel>> process_10(const vector<vector<Pixel>> &image)
{
    int height = image.size();
    int width = image[0].size();
    vector<vector<Pixel>> new_img(height, vector<Pixel>(width));
    Pixel this_pixel;

    int red;
    int green;
    int blue;
    int max_color;
    int sum_color;
    int new_red;
    int new_green;
    int new_blue;

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            this_pixel = image[row][col];
            red = this_pixel.red;
            green = this_pixel.green;
            blue = this_pixel.blue;

            max_color = rgb_max(red, green, blue);
            sum_color = red + green + blue;

            if (sum_color > 549)
            {
                new_red = 255;
                new_green = 255;
                new_blue = 255;
            }
            else if (sum_color < 151)
            {
                new_red = 0;
                new_green = 0;
                new_blue = 0;
            }
            else if (max_color == red)
            {
                new_red = 255;
                new_green = 0;
                new_blue = 0;
            }
            else if (max_color == green)
            {
                new_red = 0;
                new_green = 255;
                new_blue = 0;
            }
            else if (max_color == blue)
            {
                new_red = 0;
                new_green = 0;
                new_blue = 255;
            }
            new_img[row][col].red = new_red;
            new_img[row][col].green = new_green;
            new_img[row][col].blue = new_blue;
        }
    }
    return new_img;
}

// ________________________________________________________ Process 11 Mirror Horizontally

/**
 * Description: Flips the image horizontally to make a mirrored image
 * @param 2d vector of type Pixel
 * @return a new 2d vector of type pixel modified
 */

vector<vector<Pixel>> process_11(const vector<vector<Pixel>> &image)
{
    int height = image.size();
    int width = image[0].size();
    vector<vector<Pixel>> new_img(height, vector<Pixel>(width));

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            new_img[row][width - col - 1] = image[row][col];
        }
    }
    return new_img;
}

// ________________________________________________________ Process 12 Mirror Vertically

/**
 * Description: Flips the image vertically to make a mirrored image
 * @param 2d vector of type Pixel
 * @return a new 2d vector of type pixel modified
 */

vector<vector<Pixel>> process_12(const vector<vector<Pixel>> &image)
{
    int height = image.size();
    int width = image[0].size();
    vector<vector<Pixel>> new_img(height, vector<Pixel>(width));

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            new_img[height - row - 1][col] = image[row][col];
        }
    }
    return new_img;
}

// ________________________________________________________ Process 13 Mix 2 images

/**
 * Description: Blends two images together by averaging the pixels
 * @param 2d vector of type Pixel
 * @param 2d vector of type Pixel for image B
 * @return a new 2d vector of type pixel modified
 */

vector<vector<Pixel>> process_13(const vector<vector<Pixel>> &image, const vector<vector<Pixel>> &image_B)
{
    int height = image.size();
    int width = image[0].size();
    int height_B = image_B.size();
    int width_B = image_B[0].size();
    vector<vector<Pixel>> new_img(height, vector<Pixel>(width));

    if (height != height_B || width != width_B)
    {
        return new_img;
    }

    int red_A;
    int green_A;
    int blue_A;
    int red_B;
    int green_B;
    int blue_B;

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            red_A = image[row][col].red;
            green_A = image[row][col].green;
            blue_A = image[row][col].blue;
            red_B = image_B[row][col].red;
            green_B = image_B[row][col].green;
            blue_B = image_B[row][col].blue;

            new_img[row][col].red = (red_A + red_B) / 2;
            new_img[row][col].green = (green_A + green_B) / 2;
            new_img[row][col].blue = (blue_A + blue_B) / 2;
        }
    }
    return new_img;
}

// ________________________________________________________ Process 14 Weighted Mix 2 images

/**
 * Description: Blends two images together by computing weights and averaging the pixels
 * @param 2d vector of type Pixel
 * @param 2d vector of type Pixel for image B
 * @param floating point weight, between 0 and 1.
 * @param floating point weight, between 0 and 1.
 * @return a new 2d vector of type pixel modified
 */

vector<vector<Pixel>> process_14(const vector<vector<Pixel>> &image, const vector<vector<Pixel>> &image_B, double weight_A, double weight_B)
{
    int height = image.size();
    int width = image[0].size();
    int height_B = image_B.size();
    int width_B = image_B[0].size();
    vector<vector<Pixel>> new_img(height, vector<Pixel>(width));

    if (height != height_B || width != width_B)
    {
        return new_img;
    }

    int red_A;
    int green_A;
    int blue_A;
    int red_B;
    int green_B;
    int blue_B;

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            red_A = image[row][col].red * weight_A;
            green_A = image[row][col].green * weight_A;
            blue_A = image[row][col].blue * weight_A;
            red_B = image_B[row][col].red * weight_B;
            green_B = image_B[row][col].green * weight_B;
            blue_B = image_B[row][col].blue * weight_B;

            new_img[row][col].red = (red_A + red_B) / 2;
            new_img[row][col].green = (green_A + green_B) / 2;
            new_img[row][col].blue = (blue_A + blue_B) / 2;
        }
    }
    return new_img;
}

//***************************************************************************************************//
// HELPER FUNCTIONS FOR APPLICATION
//***************************************************************************************************//

// ________________________________________________________ Selection -> Process Image Function

/**
 * Description: Takes user selection and processes image.  Prompts user if additional info needed.
 * @param 2d vector of type Pixel
 * @param int number for selecting process
 * @return a new 2d vector of type pixel modified
 */

vector<vector<Pixel>> process_image(vector<vector<Pixel>> image, int name_idx)
{
    vector<vector<Pixel>> new_image;
    if (name_idx == 1)
    {
        new_image = process_1(image);
    }
    else if (name_idx == 2)
    {
        double scaling_factor;
        cout << "Enter Scaling Factor: ";
        cin >> scaling_factor;
        new_image = process_2(image, scaling_factor);
    }
    else if (name_idx == 3)
    {
        new_image = process_3(image);
    }
    else if (name_idx == 4)
    {
        new_image = process_4(image);
    }
    else if (name_idx == 5)
    {
        int num_rotations;
        cout << "Enter integer of 90 degree rotations: ";
        cin >> num_rotations;
        new_image = process_5(image, num_rotations);
    }
    else if (name_idx == 6)
    {
        double x_scale;
        double y_scale;
        cout << "Enter X Scale: ";
        cin >> x_scale;
        cout << "Enter Y Scale: ";
        cin >> y_scale;
        new_image = process_6(image, x_scale, y_scale);
    }
    else if (name_idx == 7)
    {
        new_image = process_7(image);
    }
    else if (name_idx == 8)
    {
        double scaling_factor;
        cout << "Enter Scaling Factor: ";
        cin >> scaling_factor;
        new_image = process_8(image, scaling_factor);
    }
    else if (name_idx == 9)
    {
        double scaling_factor;
        cout << "Enter Scaling Factor: ";
        cin >> scaling_factor;
        new_image = process_9(image, scaling_factor);
    }
    else if (name_idx == 10)
    {
        new_image = process_10(image);
    }
    else if (name_idx == 11)
    {
        new_image = process_11(image);
    }
    else if (name_idx == 12)
    {
        new_image = process_12(image);
    }
    return new_image;
}

// ________________________________________________________ Check Valid Input

/**
 * Description: Checks if the user's selection is valid
 * @param string, user's selection
 * @return true if valid selection, false if invalid.
 */

bool check_valid_input(string input)
{
    string valid_inputs[] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14"};
    for (int i = 0; i < 15; i++)
    {
        if (input == valid_inputs[i])
        {
            return true;
        }
    }
    return false;
}

// ________________________________________________________ Get filename

/**
 * Description: get's name of image for processing and returns it
 * @param
 * @return string, filename
 */

string get_filename()
{
    string filename;
    cout << "input BMP filename: ";
    cin >> filename;
    return filename;
}

// ________________________________________________________ Print Menu Options

/**
 * Description: Prints out menu options for user
 * @param string filename to insert in the first selection
 * @return
 */

void menu_options(string filename)
{
    cout << "" << endl;
    cout << "IMAGE PROCESSING MENU" << endl;
    cout << "0) Change image (current: " << filename << ")" << endl;
    cout << "1) Vignette" << endl;
    cout << "2) Clarendon" << endl;
    cout << "3) Grayscale" << endl;
    cout << "4) Rotate 90 degrees" << endl;
    cout << "5) Rotate multiple 90 degrees" << endl;
    cout << "6) Enlarge" << endl;
    cout << "7) High contrast" << endl;
    cout << "8) Lighten" << endl;
    cout << "9) Darken" << endl;
    cout << "10) Black, white, red, green, blue" << endl;
    cout << "11) Mirror Horizontally" << endl;
    cout << "12) Mirror Vertically" << endl;
    cout << "13) Blend 2 images" << endl;
    cout << "14) Weighted Blend of 2 images" << endl;
    cout << "" << endl;
    cout << "Enter menu selection (Q to quit): ";
}
//***************************************************************************************************//
// Application
//***************************************************************************************************//

/**
 * Description: Handles important variables, control flow for implementing application with helper
 * function calls and reading/writing image.
 * @param
 * @return string with success or failure message
 */

string application()
{

    string filename;
    string selection;
    int name_idx;
    string output_name;

    string process_names[] = {"rename file", "Vignette", "Clarendon", "Grayscale", "Rotate 90 degrees", "Rotate multiple 90 degrees", "Enlarge", "High contrast", "Lighten", "Darken", "Black, white, red, green, blue", "Mirror Horizontally", "Mirror Vertically", "Blend 2 images", "Weighted blend of images"};

    filename = get_filename();

    menu_options(filename);

    cin >> selection;

    if (selection == "Q")
    {
        return "\nThank you for using my program!\nQuitting... \n\n";
    }
    if (check_valid_input(selection) == false)
    {
        cout << "Please provide valid input!" << endl;
        return application();
    }

    if (selection == "0")
    {
        cout << "Change image Selected!";
        return application();
    }

    if (check_valid_input(selection))
    {
        name_idx = stoi(selection);
        cout << process_names[name_idx] << " selected" << endl;
    }
    // ________________________________________________________________ optional stuff starts here
    if (name_idx == 13)
    {
        string filename_B;
        cout << "input BMP filename for 2nd image, must match size! ";
        cin >> filename_B;
        cout << "Enter output BMP filename: ";
        cin >> output_name;
        vector<vector<Pixel>> image = read_image(filename);
        vector<vector<Pixel>> image_B = read_image(filename_B);
        if (image.size() != image_B.size() || image[0].size() != image_B[0].size())
        {
            cout << "image sizes do not match!!!";
            application();
        }
        vector<vector<Pixel>> new_image = process_13(image, image_B);
        write_image(output_name, new_image);
        return "Successfully applied " + process_names[name_idx] + "!";
    }
    if (name_idx == 14)
    {
        double weight_A;
        cout << "input weight for first image, weights must add to 1: ";
        cin >> weight_A;
        string filename_B;
        cout << "input BMP filename for 2nd image, must match size! ";
        cin >> filename_B;
        double weight_B;
        cout << "input weight for second image, weights must add to 1: ";
        cin >> weight_B;

        if (weight_A + weight_B != 1.0)
        {
            cout << "weights do not equal 1!\n\n";
            return application();
        }

        cout << "Enter output BMP filename: ";
        cin >> output_name;
        vector<vector<Pixel>> image = read_image(filename);
        vector<vector<Pixel>> image_B = read_image(filename_B);
        if (image.size() != image_B.size() || image[0].size() != image_B[0].size())
        {
            cout << "image sizes do not match!!!";
            application();
        }
        vector<vector<Pixel>> new_image = process_14(image, image_B, weight_A, weight_B);
        write_image(output_name, new_image);
        return "Successfully applied " + process_names[name_idx] + "!";
    }
    // ________________________________________________________________ optional stuff ends here

    cout << "Enter output BMP filename: ";
    cin >> output_name;

    vector<vector<Pixel>> image = read_image(filename);

    vector<vector<Pixel>> new_image = process_image(image, name_idx);

    write_image(output_name, new_image);

    return "Successfully applied " + process_names[name_idx] + "!";
}

// ________________________________________________________ MAIN FUNCTION

int main()
{

    cout << "CSPB 1300 Image Processing Application" << endl;
    string message = application();
    cout << message;

    return 0;
}

/*
    g++ -std=c++11 -o main main.cpp
    ./main

    g++ -std=c++11 -o main main.cpp && ./main
*/

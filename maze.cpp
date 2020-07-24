#include <iostream>
#include <vector>
#include <stack>
#include <queue>
#include <string>
#include <ctime>

struct MyMaze
{
private:
    struct MyCell
    {
        bool wall_right = true;
        bool wall_down = true;
    };
    std::vector<std::vector<MyCell> > g;
    std::vector<std::pair<int, int> > path;
    int rows;
    int cols;

    bool printTime = true;

    std::pair<int, int> entry = std::make_pair(-1, -1);
    std::pair<int, int> exit = std::make_pair(-1, -1);
    bool entryAndExitExist = false;

    const int defaultSize = 20;

    std::vector<unsigned char> wallColor{ 0,0,0 };
    std::vector<unsigned char> backgroundColor{ 255,255,255 };
    std::vector<unsigned char> pathColor{ 100,100,255 };

    const int bytesPerPixel = 3; /// red, green, blue
    const int fileHeaderSize = 14;
    const int infoHeaderSize = 40;

    void generateBitmapImage(unsigned char *image, int height, int width, const char* imageFileName) {

        unsigned char padding[3] = { 0, 0, 0 };
        int paddingSize = (4 - (width*bytesPerPixel) % 4) % 4;

        unsigned char* fileHeader = createBitmapFileHeader(height, width, paddingSize);
        unsigned char* infoHeader = createBitmapInfoHeader(height, width);

        FILE* imageFile = fopen(imageFileName, "wb");

        fwrite(fileHeader, 1, fileHeaderSize, imageFile);
        fwrite(infoHeader, 1, infoHeaderSize, imageFile);

        int i;
        for (i = 0; i < height; i++) {
            //inverse height
            fwrite(image + ((height - i - 1)*width*bytesPerPixel), bytesPerPixel, width, imageFile);
            fwrite(padding, 1, paddingSize, imageFile);
        }

        fclose(imageFile);
    }

    unsigned char* createBitmapFileHeader(int height, int width, int paddingSize) {
        int fileSize = fileHeaderSize + infoHeaderSize + (bytesPerPixel*width + paddingSize) * height;

        static unsigned char fileHeader[] = {
            0,0, /// signature
            0,0,0,0, /// image file size in bytes
            0,0,0,0, /// reserved
            0,0,0,0, /// start of pixel array
        };

        fileHeader[0] = (unsigned char)('B');
        fileHeader[1] = (unsigned char)('M');
        fileHeader[2] = (unsigned char)(fileSize);
        fileHeader[3] = (unsigned char)(fileSize >> 8);
        fileHeader[4] = (unsigned char)(fileSize >> 16);
        fileHeader[5] = (unsigned char)(fileSize >> 24);
        fileHeader[10] = (unsigned char)(fileHeaderSize + infoHeaderSize);

        return fileHeader;
    }

    unsigned char* createBitmapInfoHeader(int height, int width) {
        static unsigned char infoHeader[] = {
            0,0,0,0, /// header size
            0,0,0,0, /// image width
            0,0,0,0, /// image height
            0,0, /// number of color planes
            0,0, /// bits per pixel
            0,0,0,0, /// compression
            0,0,0,0, /// image size
            0,0,0,0, /// horizontal resolution
            0,0,0,0, /// vertical resolution
            0,0,0,0, /// colors in color table
            0,0,0,0, /// important color count
        };

        infoHeader[0] = (unsigned char)(infoHeaderSize);
        infoHeader[4] = (unsigned char)(width);
        infoHeader[5] = (unsigned char)(width >> 8);
        infoHeader[6] = (unsigned char)(width >> 16);
        infoHeader[7] = (unsigned char)(width >> 24);
        infoHeader[8] = (unsigned char)(height);
        infoHeader[9] = (unsigned char)(height >> 8);
        infoHeader[10] = (unsigned char)(height >> 16);
        infoHeader[11] = (unsigned char)(height >> 24);
        infoHeader[12] = (unsigned char)(1);
        infoHeader[14] = (unsigned char)(bytesPerPixel * 8);

        return infoHeader;
    }

    void findPath(std::pair<int, int> from, std::pair<int, int> to)
    {
        int n = cols * rows;

        std::queue< std::pair<int, int> > q;
        q.push(from);
        std::vector<std::vector<bool>> used(rows, std::vector<bool>(cols, 0));
        std::vector<std::vector< std::pair<int, int> >  > parent(rows, std::vector < std::pair<int, int> >(cols));

        used[from.first][from.second] = 1;

        std::pair<int, int> root = std::make_pair(-1, -1);
        parent[from.first][from.second] = root;

        while (!q.empty())
        {
            std::pair<int, int> cur = q.front();
            q.pop();

            //upper is not visited
            if (cur.first - 1 >= 0 && !g[cur.first - 1][cur.second].wall_down && !used[cur.first - 1][cur.second])
            {
                used[cur.first - 1][cur.second] = 1;
                q.push(std::make_pair(cur.first - 1, cur.second));
                parent[cur.first - 1][cur.second] = cur;
            }
            //bottom is not visited
            if (cur.first + 1 < rows && !g[cur.first][cur.second].wall_down && !used[cur.first + 1][cur.second])
            {
                used[cur.first + 1][cur.second] = 1;
                q.push(std::make_pair(cur.first + 1, cur.second));
                parent[cur.first + 1][cur.second] = cur;
            }
            //left is not visited
            if (cur.second - 1 >= 0 && !g[cur.first][cur.second - 1].wall_right && !used[cur.first][cur.second - 1])
            {
                used[cur.first][cur.second - 1] = 1;
                q.push(std::make_pair(cur.first, cur.second - 1));
                parent[cur.first][cur.second - 1] = cur;
            }
            //right is not visited
            if (cur.second + 1 < cols && !g[cur.first][cur.second].wall_right && !used[cur.first][cur.second + 1])
            {
                used[cur.first][cur.second + 1] = 1;
                q.push(std::make_pair(cur.first, cur.second + 1));
                parent[cur.first][cur.second + 1] = cur;
            }
        }

        for (std::pair<int, int> cur = to; cur != root; cur = parent[cur.first][cur.second])
            path.push_back(cur);
        reverse(path.begin(), path.end());
    }

    void markEntryAndExit(unsigned char * imageAsArray, const int wallWidth, const int height, const int width,
        const std::vector<unsigned char> color)
    {
        //entry on top side
        if (entry.first == 0)
        {
            int start_down = 0;
            int start_right = wallWidth + wallWidth * 2 * entry.second;
            int end_down = wallWidth * 2;
            int end_rigth = wallWidth + wallWidth * 2 * entry.second + wallWidth;

            for (int i_imag = start_down; i_imag < end_down; i_imag++)
            {
                for (int j_imag = start_right; j_imag < end_rigth; j_imag++)
                {
                    memcpy((imageAsArray + (width*(i_imag)+j_imag) * 3), color.data(), 3 * sizeof(unsigned char));
                }
            }
        }
        //entry on left side
        else if (entry.second == 0)
        {
            int start_down = wallWidth + wallWidth * 2 * entry.first;
            int start_right = 0;
            int end_down = start_down + wallWidth;
            int end_rigth = start_right + wallWidth * 2;

            for (int i_imag = start_down; i_imag < end_down; i_imag++)
            {
                for (int j_imag = start_right; j_imag < end_rigth; j_imag++)
                {
                    memcpy((imageAsArray + (width*(i_imag)+j_imag) * 3), color.data(), 3 * sizeof(unsigned char));
                }
            }
        }
        //entry on bottom side
        else if (entry.first == rows - 1)
        {
            int end_down = height;
            int start_down = height - wallWidth * 2;
            int start_right = wallWidth + wallWidth * 2 * entry.second;
            int end_rigth = start_right + wallWidth;

            for (int i_imag = start_down; i_imag < end_down; i_imag++)
            {
                for (int j_imag = start_right; j_imag < end_rigth; j_imag++)
                {
                    memcpy((imageAsArray + (width*(i_imag)+j_imag) * 3), color.data(), 3 * sizeof(unsigned char));
                }
            }
        }
        //entry on right side
        else if (entry.second == cols - 1)
        {
            int start_down = wallWidth + wallWidth * 2 * entry.first;
            int end_rigth = width;
            int start_right = width - wallWidth * 2;
            int end_down = start_down + wallWidth;

            for (int i_imag = start_down; i_imag < end_down; i_imag++)
            {
                for (int j_imag = start_right; j_imag < end_rigth; j_imag++)
                {
                    memcpy((imageAsArray + (width*(i_imag)+j_imag) * 3), color.data(), 3 * sizeof(unsigned char));
                }
            }
        }

        //exit on bottom side
        if (exit.first == rows - 1)
        {
            int end_down = height;
            int start_down = height - wallWidth;
            int start_right = wallWidth + wallWidth * 2 * exit.second;
            int end_rigth = start_right + wallWidth;

            for (int i_imag = start_down; i_imag < end_down; i_imag++)
            {
                for (int j_imag = start_right; j_imag < end_rigth; j_imag++)
                {
                    memcpy((imageAsArray + (width*(i_imag)+j_imag) * 3), color.data(), 3 * sizeof(unsigned char));
                }
            }
        }
        //exit on right side
        else if (exit.second == cols - 1)
        {
            int start_down = wallWidth + wallWidth * 2 * exit.first;
            int end_rigth = width;
            int start_right = width - wallWidth;
            int end_down = start_down + wallWidth;

            for (int i_imag = start_down; i_imag < end_down; i_imag++)
            {
                for (int j_imag = start_right; j_imag < end_rigth; j_imag++)
                {
                    memcpy((imageAsArray + (width*(i_imag)+j_imag) * 3), color.data(), 3 * sizeof(unsigned char));
                }
            }
        }
        //exit on left side
        else if (exit.second == 0)
        {
            int start_down = wallWidth + wallWidth * 2 * exit.first;
            int start_right = 0;
            int end_down = start_down + wallWidth;
            int end_rigth = start_right + wallWidth;

            for (int i_imag = start_down; i_imag < end_down; i_imag++)
            {
                for (int j_imag = start_right; j_imag < end_rigth; j_imag++)
                {
                    memcpy((imageAsArray + (width*(i_imag)+j_imag) * 3), color.data(), 3 * sizeof(unsigned char));
                }
            }
        }
        //exit on top side
        else if (exit.first == 0)
        {
            int start_down = 0;
            int start_right = wallWidth + wallWidth * 2 * exit.second;
            int end_down = wallWidth;
            int end_rigth = wallWidth + wallWidth * 2 * exit.second + wallWidth;

            for (int i_imag = start_down; i_imag < end_down; i_imag++)
            {
                for (int j_imag = start_right; j_imag < end_rigth; j_imag++)
                {
                    memcpy((imageAsArray + (width*(i_imag)+j_imag) * 3), color.data(), 3 * sizeof(unsigned char));
                }
            }
        }
    }

public:
    MyMaze(int rows, int cols)
    {
        g.resize(rows, std::vector<MyCell>(cols));

        this->rows = rows;
        this->cols = cols;
    }

    MyMaze()
    {
        g.resize(defaultSize, std::vector<MyCell>(defaultSize));

        this->rows = defaultSize;
        this->cols = defaultSize;
    }

    MyMaze(int rows, int cols, bool printTime)
    {
        g.resize(rows, std::vector<MyCell>(cols));

        this->rows = rows;
        this->cols = cols;
        this->printTime = printTime;

    }

    void buildWithRecursiveBacktracker(std::pair<int, int> entry, std::pair<int, int> exit)
    {

        clock_t start = clock();

        std::vector<std::vector<bool> > used(rows, std::vector<bool>(cols, 0));
        int x_start = rand() % rows;
        int y_start = rand() % cols;

        used[x_start][y_start] = 1;
        std::stack<std::pair<int, int> > toUse;
        toUse.push(std::make_pair(x_start, y_start));

        while (!toUse.empty())
        {
            std::pair<int, int> cur = toUse.top();
            toUse.pop();

            std::vector<std::pair<int, int>> unusedNeighbors;
            //upper is not visited
            if (cur.first - 1 >= 0 && !used[cur.first - 1][cur.second])
            {
                unusedNeighbors.push_back(std::make_pair(cur.first - 1, cur.second));
            }
            //bottom is not visited
            if (cur.first + 1 < rows && !used[cur.first + 1][cur.second])
            {
                unusedNeighbors.push_back(std::make_pair(cur.first + 1, cur.second));
            }
            //left is not visited
            if (cur.second - 1 >= 0 && !used[cur.first][cur.second - 1])
            {
                unusedNeighbors.push_back(std::make_pair(cur.first, cur.second - 1));
            }
            //right is not visited
            if (cur.second + 1 < cols && !used[cur.first][cur.second + 1])
            {
                unusedNeighbors.push_back(std::make_pair(cur.first, cur.second + 1));
            }

            //unvisited neeighbors exist
            if (unusedNeighbors.size() > 0)
            {
                //Push the current cell to the stack
                toUse.push(cur);
                //Choose one of the unvisited neighbours
                std::pair<int, int> next = unusedNeighbors[rand() % unusedNeighbors.size()];

                //Remove the wall between the current cell and the chosen cell
                //if upper
                if (next.first < cur.first)
                {
                    g[next.first][next.second].wall_down = false;
                }
                //if bottom
                else if (next.first > cur.first)
                {
                    g[cur.first][cur.second].wall_down = false;
                }
                //if left
                else if (next.second < cur.second)
                {
                    g[next.first][next.second].wall_right = false;
                }
                //if bottom
                else if (next.second > cur.second)
                {
                    g[cur.first][cur.second].wall_right = false;
                }

                //Mark the chosen cell as visited and push it to the stack
                used[next.first][next.second] = true;
                toUse.push(next);
            }
        }

        if (entry.first < 0) entry.first = 0;
        if (entry.first >= rows) entry.first = rows - 1;
        if (exit.first < 0) exit.first = 0;
        if (exit.first >= rows) exit.first = rows - 1;

        if (entry.second < 0) entry.second = 0;
        if (entry.second >= cols) entry.second = cols - 1;
        if (exit.second < 0) exit.second = 0;
        if (exit.second >= cols) exit.second = cols - 1;

        this->entry = entry;
        this->exit = exit;
        entryAndExitExist = true;

        if (printTime)
            std::cout << "Maze genaration time: " << double(clock() - start) / CLK_TCK << std::endl;
    }

    void buildWithRecursiveBacktracker()
    {
        buildWithRecursiveBacktracker(std::make_pair(0, rand() % cols), std::make_pair(rows - 1, rand() % cols));
    }

    void print()
    {
        const char c_wall = '|';
        const char c_floor = '_';

        for (int j = 0; j < 2 * cols + 1; j++)
            std::cout << c_floor;

        std::cout << std::endl;
        for (int i = 0; i < rows; i++)
        {
            std::cout << c_wall;
            for (int j = 0; j < cols; j++)
            {
                std::cout << (g[i][j].wall_down ? c_floor : ' ');
                std::cout << (g[i][j].wall_right ? c_wall : ' ');
            }
            std::cout << std::endl;
        }
    }

    void setDrawWallColor(unsigned char red, unsigned char green, unsigned char blue)
    {
        wallColor = { (blue < 256) ? blue : (unsigned char)255,
            (green < 256) ? green : (unsigned char)255,
            (red < 256) ? red : (unsigned char)255 };
    }

    void setDrawBackgroundColor(unsigned char red, unsigned char green, unsigned char blue)
    {
        backgroundColor = { (blue < 256) ? blue : (unsigned char)255,
            (green < 256) ? green : (unsigned char)255,
            (red < 256) ? red : (unsigned char)255 };
    }

    void setDrawPathColor(unsigned char red, unsigned char green, unsigned char blue)
    {
        pathColor = { (blue < 256) ? blue : (unsigned char)255,
            (green < 256) ? green : (unsigned char)255,
            (red < 256) ? red : (unsigned char)255 };
    }

    void drawAsBMP(int wallWidth, std::string name, bool solution)
    {
        clock_t start = clock();

        int height = rows * wallWidth * 2 + wallWidth;
        int width = cols * wallWidth * 2 + wallWidth;
        unsigned char * imageAsArray = new unsigned char[height*width * 3];

        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                memcpy((imageAsArray + (width*i + j) * 3), backgroundColor.data(), 3 * sizeof(unsigned char));
            }
        }

        //left and top wall
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < wallWidth; j++)
            {
                memcpy((imageAsArray + (width*i + j) * 3), wallColor.data(), 3 * sizeof(unsigned char));
            }
        }
        for (int i = 0; i < wallWidth; i++)
        {
            for (int j = 0; j < width; j++)
            {
                memcpy((imageAsArray + (width*i + j) * 3), wallColor.data(), 3 * sizeof(unsigned char));
            }
        }

        //check every cell and draw its if they exist
        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                if (g[i][j].wall_right)
                {
                    int start_down = wallWidth * 2 * i;
                    int start_right = wallWidth * 2 + wallWidth * 2 * j;
                    int end_down = start_down + wallWidth * 3;
                    int end_rigth = start_right + wallWidth;

                    for (int i_imag = start_down; i_imag < end_down; i_imag++)
                    {
                        for (int j_imag = start_right; j_imag < end_rigth; j_imag++)
                        {
                            memcpy((imageAsArray + (width*(i_imag)+j_imag) * 3), wallColor.data(), 3 * sizeof(unsigned char));
                        }
                    }
                }
                if (g[i][j].wall_down)
                {
                    int start_down = wallWidth * 2 + wallWidth * 2 * i;
                    int start_right = wallWidth * 2 * j;
                    int end_down = start_down + wallWidth;
                    int end_rigth = start_right + wallWidth * 3;

                    for (int i_imag = start_down; i_imag < end_down; i_imag++)
                    {
                        for (int j_imag = start_right; j_imag < end_rigth; j_imag++)
                        {
                            memcpy((imageAsArray + (width*(i_imag)+j_imag) * 3), wallColor.data(), 3 * sizeof(unsigned char));
                        }
                    }
                }
            }
        }

        if (entryAndExitExist)
        {
            markEntryAndExit(imageAsArray, wallWidth, height, width, backgroundColor);
        }

        if (printTime)
            std::cout << "Maze image generation time: " << double(clock() - start) / CLK_TCK << std::endl;

        start = clock();
        generateBitmapImage(imageAsArray, height, width, (name + ".bmp").c_str());
        if (printTime)
            std::cout << "Maze image write time: " << double(clock() - start) / CLK_TCK << std::endl;

        if (solution)
        {
            start = clock();

            findPath(entry, exit);

            for (int i = 0; i < path.size() - 1; i++)
            {
                int start_down = std::min(wallWidth * 2 * path[i].first + wallWidth, wallWidth * 2 * path[i + 1].first + wallWidth);
                int start_right = std::min(wallWidth + wallWidth * 2 * path[i].second, wallWidth + wallWidth * 2 * path[i + 1].second);
                int end_down = std::max(wallWidth * 2 * path[i].first + wallWidth, wallWidth * 2 * path[i + 1].first + wallWidth * 2);
                int end_rigth = std::max(wallWidth + wallWidth * 2 * path[i].second, wallWidth + wallWidth * 2 * path[i + 1].second + wallWidth);

                for (int i_imag = start_down; i_imag < end_down; i_imag++)
                {
                    for (int j_imag = start_right; j_imag < end_rigth; j_imag++)
                    {
                        memcpy((imageAsArray + (width*(i_imag)+j_imag) * 3), pathColor.data(), 3 * sizeof(unsigned char));
                    }
                }
            }

            markEntryAndExit(imageAsArray, wallWidth, height, width, pathColor);

            if (printTime)
                std::cout << "Solution image generation time: " << double(clock() - start) / CLK_TCK << std::endl;

            start = clock();
            generateBitmapImage(imageAsArray, height, width, (name + "_solution.bmp").c_str());
            if (printTime)
                std::cout << "Solution image write time: " << double(clock() - start) / CLK_TCK << std::endl;
        }
    }

    int Rows()
    {
        return rows;
    }

    int Cols()
    {
        return cols;
    }
};

void help()
{
    std::cout << "Arguments usage: " << std::endl;
    std::cout << "  -h                      print this help" << std::endl;
    std::cout << "  -rows <value>           set maze number of rows" << std::endl;
    std::cout << "                          default - 25" << std::endl;
    std::cout << "  -cols <value>           set maze number of columns" << std::endl;
    std::cout << "                          default - 25" << std::endl;
    std::cout << "  -o <name>               set output .bmp file name" << std::endl;
    std::cout << "                          default - Output" << std::endl;
    std::cout << "  -entry <row> <col>      set maze entrance coords" << std::endl;
    std::cout << "                          default - random coords" << std::endl;
    std::cout << "  -exit <row> <col>       set maze exit coords" << std::endl;
    std::cout << "                          default - random coords" << std::endl;
    std::cout << "  -solution               create second .bmp image with path from entry to exit" << std::endl;
    std::cout << "                          without this flag creates no solution" << std::endl;
    std::cout << "  -noTime                 do not print program run time " << std::endl;
    std::cout << "  -console                print maze in console with ASCII symbols" << std::endl;
    std::cout << "  -wallW <value>          set wall widht in pixels" << std::endl;
    std::cout << "                          default - 5" << std::endl;
    std::cout << "  -wallC <r> <g> <b>      set wall color" << std::endl;
    std::cout << "                          default - 0 0 0" << std::endl;
    std::cout << "  -backC <r> <g> <b>      set background color" << std::endl;
    std::cout << "                          default - 255 255 255" << std::endl;
    std::cout << "  -pathC <r> <g> <b>      set path from entry to exit color" << std::endl;
    std::cout << "                          default - 100 255 100" << std::endl;

}

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        if (std::string(argv[1]) == "-h")
        {
            help();
            return 0;
        }
        int rows = 25;
        int cols = 25;
        int wallWidht = 5;
        std::string outName = "Output";
        std::pair<int, int> entry = std::make_pair(0, rand() % cols);
        std::pair<int, int> exit = std::make_pair(rows - 1, rand() % cols);

        int wall_r = 0, wall_g = 0, wall_b = 0;
        int background_r = 255, background_g = 255, background_b = 255;
        int path_r = 100, path_g = 255, path_b = 100;

        bool solution = false;
        bool console = false;
        bool printTime = true;

        for (int i = 0; i < argc; i++)
        {
            if (std::string(argv[i]) == "-rows")
            {
                rows = atoi(argv[++i]);
                entry = std::make_pair(0, rand() % cols);
                exit = std::make_pair(rows - 1, rand() % cols);
            }
            else if (std::string(argv[i]) == "-cols")
            {
                cols = atoi(argv[++i]);
                entry = std::make_pair(0, rand() % cols);
                exit = std::make_pair(rows - 1, rand() % cols);
            }
            else if (std::string(argv[i]) == "-o")
            {
                outName = argv[++i];
            }
            else if (std::string(argv[i]) == "-entry")
            {
                int a = atoi(argv[++i]);
                int b = atoi(argv[++i]);
                entry = std::make_pair(a, b);
            }
            else if (std::string(argv[i]) == "-exit")
            {
                int a = atoi(argv[++i]);
                int b = atoi(argv[++i]);
                exit = std::make_pair(a, b);
            }
            else if (std::string(argv[i]) == "-solution")
            {
                solution = true;
            }
            else if (std::string(argv[i]) == "-console")
            {
                console = true;
            }
            else if (std::string(argv[i]) == "-wallW")
            {
                wallWidht = atoi(argv[++i]);
            }
            else if (std::string(argv[i]) == "-wallC")
            {
                wall_r = atoi(argv[++i]);
                wall_g = atoi(argv[++i]);
                wall_b = atoi(argv[++i]);
            }
            else if (std::string(argv[i]) == "-backC")
            {
                background_r = atoi(argv[++i]);
                background_g = atoi(argv[++i]);
                background_b = atoi(argv[++i]);
            }
            else if (std::string(argv[i]) == "-pathC")
            {
                path_r = atoi(argv[++i]);
                path_g = atoi(argv[++i]);
                path_b = atoi(argv[++i]);
            }
            else if (std::string(argv[i]) == "-noTime")
            {
                printTime = false;
            }

        }
        clock_t start = clock();
        srand(time(NULL));
        MyMaze a(rows, cols, printTime);

        a.buildWithRecursiveBacktracker(entry, exit);

        if (console)
        {
            a.print();
        }
        else
        {
            a.setDrawBackgroundColor(background_r, background_g, background_b);
            a.setDrawWallColor(wall_r, wall_g, wall_b);
            a.setDrawPathColor(path_r, path_g, path_b);
            a.drawAsBMP(wallWidht, outName, solution);
            if (printTime)
                std::cout << std::endl << "Total time: " << double(clock() - start) / CLK_TCK << std::endl;
        }
    }
    else
    {
        clock_t start = clock();
        srand(time(NULL));

        MyMaze a(25, 25);

        a.buildWithRecursiveBacktracker();
        a.print();

        std::cout << std::endl << "Total time: " << double(clock() - start) / CLK_TCK << std::endl;
    }
}
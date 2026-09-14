#include <iostream>
#include <queue>
#include <string>
#include <vector>

// Grid Dimensions
const int MAZE_SIZE = 16;

// Direction Vectors: 0 = North, 1 = East, 2 = South, 3 = West
const int DX[4] = {0, 1, 0, -1};
const int DY[4] = {1, 0, -1, 0};

// Orientation state: starts at (0,0) facing North
int current_x = 0;
int current_y = 0;
int current_dir = 0; 

// Distance Matrix
int dist[MAZE_SIZE][MAZE_SIZE];

// Wall Matrix: walls[x][y][dir] is true if a wall exists in direction dir
bool walls[MAZE_SIZE][MAZE_SIZE][4];

// API Wrappers for MMS Simulator
bool API_wallFront() {
    std::cout << "wallFront" << std::endl;
    std::string response;
    std::cin >> response;
    return response == "true";
}

bool API_wallLeft() {
    std::cout << "wallLeft" << std::endl;
    std::string response;
    std::cin >> response;
    return response == "true";
}

bool API_wallRight() {
    std::cout << "wallRight" << std::endl;
    std::string response;
    std::cin >> response;
    return response == "true";
}

void API_moveForward() {
    std::cout << "moveForward" << std::endl;
    std::string ack;
    std::cin >> ack;
    
    current_x += DX[current_dir];
    current_y += DY[current_dir];
}

void API_turnLeft() {
    std::cout << "turnLeft" << std::endl;
    std::string ack;
    std::cin >> ack;
    current_dir = (current_dir + 3) % 4;
}

void API_turnRight() {
    std::cout << "turnRight" << std::endl;
    std::string ack;
    std::cin >> ack;
    current_dir = (current_dir + 1) % 4;
}

void API_setText(int x, int y, const std::string& text) {
    std::cout << "setText " << x << " " << y << " " << text << std::endl;
}

void API_setWall(int x, int y, char direction) {
    std::cout << "setWall " << x << " " << y << " " << direction << std::endl;
}

// Helper function to mark walls for both current cell and neighboring cell
void setWall(int x, int y, int dir) {
    if (walls[x][y][dir]) return; // Already set

    walls[x][y][dir] = true;

    // Draw wall visually in MMS
    char dir_chars[4] = {'n', 'e', 's', 'w'};
    API_setWall(x, y, dir_chars[dir]);

    // Mirror wall on neighboring cell's opposite side
    int nx = x + DX[dir];
    int ny = y + DY[dir];
    if (nx >= 0 && nx < MAZE_SIZE && ny >= 0 && ny < MAZE_SIZE) {
        walls[nx][ny][(dir + 2) % 4] = true;
    }
}

// Flood Fill Algorithm (Wavefront BFS)
void updateDistances() {
    // Reset all distances to maximum default
    for (int x = 0; x < MAZE_SIZE; ++x) {
        for (int y = 0; y < MAZE_SIZE; ++y) {
            dist[x][y] = 999;
        }
    }

    std::queue<std::pair<int, int>> q;

    // Target center goal cells: (7,7), (7,8), (8,7), (8,8)
    int goals[4][2] = {{7, 7}, {7, 8}, {8, 7}, {8, 8}};
    for (auto& g : goals) {
        dist[g[0]][g[1]] = 0;
        q.push({g[0], g[1]});
    }

    // Propagate distance values outward from center goal
    while (!q.empty()) {
        auto [cx, cy] = q.front();
        q.pop();

        for (int d = 0; d < 4; ++d) {
            if (!walls[cx][cy][d]) {
                int nx = cx + DX[d];
                int ny = cy + DY[d];

                if (nx >= 0 && nx < MAZE_SIZE && ny >= 0 && ny < MAZE_SIZE) {
                    if (dist[nx][ny] > dist[cx][cy] + 1) {
                        dist[nx][ny] = dist[cx][cy] + 1;
                        q.push({nx, ny});
                    }
                }
            }
        }
    }

    // Refresh display values on MMS visual grid
    for (int x = 0; x < MAZE_SIZE; ++x) {
        for (int y = 0; y < MAZE_SIZE; ++y) {
            if (dist[x][y] != 999) {
                API_setText(x, y, std::to_string(dist[x][y]));
            }
        }
    }
}

int main() {
    // Set outer boundary boundaries
    for (int i = 0; i < MAZE_SIZE; ++i) {
        setWall(i, 0, 2);             // South boundary
        setWall(i, MAZE_SIZE - 1, 0); // North boundary
        setWall(0, i, 3);             // West boundary
        setWall(MAZE_SIZE - 1, i, 1); // East boundary
    }

    updateDistances();

    while (true) {
        // Step 1: Read sensors and update local + neighbor walls
        if (API_wallFront()) {
            setWall(current_x, current_y, current_dir);
        }
        if (API_wallLeft()) {
            setWall(current_x, current_y, (current_dir + 3) % 4);
        }
        if (API_wallRight()) {
            setWall(current_x, current_y, (current_dir + 1) % 4);
        }

        // Step 2: Recalculate flood values across maze
        updateDistances();

        // Check if central goal cell is reached
        if (dist[current_x][current_y] == 0) {
            break;
        }

        // Step 3: Identify accessible neighboring cell with absolute minimum distance value
        int best_dir = -1;
        int min_dist = 999;

        for (int d = 0; d < 4; ++d) {
            if (!walls[current_x][current_y][d]) {
                int nx = current_x + DX[d];
                int ny = current_y + DY[d];
                if (nx >= 0 && nx < MAZE_SIZE && ny >= 0 && ny < MAZE_SIZE) {
                    if (dist[nx][ny] < min_dist) {
                        min_dist = dist[nx][ny];
                        best_dir = d;
                    }
                }
            }
        }

        // Step 4: Rotate mouse toward target direction and advance forward
        if (best_dir != -1) {
            int turn_offset = (best_dir - current_dir + 4) % 4;
            if (turn_offset == 1) {
                API_turnRight();
            } else if (turn_offset == 2) {
                API_turnRight();
                API_turnRight();
            } else if (turn_offset == 3) {
                API_turnLeft();
            }
            API_moveForward();
        }
    }

    return 0;
}
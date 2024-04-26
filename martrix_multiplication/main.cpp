#include <vector>
#include <iostream>
#include <mutex>
#include <memory>
#include <thread>
#include <functional>
#include <sstream>

struct MatrixCell
{
    int y, x;
};

std::ostream& operator<<(std::ostream& os, const MatrixCell& mat)
{
    return os << "(" << mat.y << ", " << mat.x << ")";
}

class Matrix
{
private:
    int m_width, m_height;
    std::vector<std::vector<int>> m_matrix;
public:
    Matrix(int width, int height) : m_width{width}, m_height{height} 
    {
        m_matrix.resize(height);
        for (size_t i = 0; i < m_matrix.size(); i++)
        {
            m_matrix[i].resize(width);
        }
    }

    int width() const {
        return m_width;
    }

    int height() const {
        return m_height;
    }

    int get(MatrixCell cell) const {
        return m_matrix[cell.y][cell.x];
    }

    void set(MatrixCell cell, int value) {
        m_matrix[cell.y][cell.x] = value;
    }

};


std::ostream& operator<<(std::ostream& os, const Matrix& mat)
{
    for (int i = 0; i < mat.height(); i++)
    {
        for (int j = 0; j < mat.width(); j++)
        {
            int value = mat.get({i, j});
            os << value << '\t';
        }
        os << std::endl;
    }
    
    return os;
}

class MatrixCellCounter
{
    int m_width, m_height;
    int m_counter = 0;
    std::mutex m_mutex;
public:
    MatrixCellCounter(int width, int height) : m_width(width), m_height(height)
    {
    }

    MatrixCell get_and_increase() {
        MatrixCell current_cell;

        {
            std::lock_guard<std::mutex> guard(m_mutex);

            current_cell.x = m_counter % m_width;
            current_cell.y = m_counter/m_width;

            m_counter++;
        }

        if(current_cell.y >= m_height) {
            current_cell.x = -1;
            current_cell.y = -1;
        }

        return current_cell;
    }
};


void print_message(const std::string& mess) {
    std::cout << mess << std::endl;
}

bool get_values(int& height, int& common_side, int& width, int& threads_count, int argc, char* argv[])
{
    if (argc < 5) {
        std::cout << "Enter all required arguments: height, common_side, width, number of threads" << std::endl;
        return false;
    }

    height = atoi(argv[1]);
    if(!height) {
        print_message("Failed to convert n");
        return false; 
    }

    common_side = atoi(argv[2]);
    if(!common_side) {
        print_message("Failed to convert m");
        return false;
    }

    width = atoi(argv[3]);
    if(!width) {
        print_message("Failed to convert k");
        return false;
    }

    threads_count = atoi(argv[4]);
    if(!threads_count) {
        print_message("Failed to convert number of threads");
        return false;
    }

    return true;
}

void fill_matrix(Matrix& matrix) {
    for(int i = 0; i < matrix.height(); i++) 
        for(int j = 0; j < matrix.width(); j++) {
            int value = rand() % 10;
            matrix.set({i, j}, value);
        }
}

std::pair<Matrix, Matrix> generate_matrices(int height, int common_side, int width) {
    Matrix left{common_side, height};
    Matrix right{width, common_side};

    fill_matrix(left);
    fill_matrix(right);

    return {left, right};
}

int calculate_matrix_cell(MatrixCell cell, const Matrix& left, const Matrix& right)
{
    int result = 0;
    int left_height = cell.y;
    int right_width = cell.x;
    for (int i = 0; i < left.width(); i++)
    {
        result += left.get({left_height, i}) * right.get({i, right_width});
    }

    return result;
}

void launch_threads_and_wait(int num_of_threads, std::function<void()> worker_function)
{
    std::vector<std::thread> threads;
    threads.resize(num_of_threads-1);
    for (size_t i = 0; i < num_of_threads-1; i++)
    {
        threads[i] = std::thread{worker_function};
    }

    worker_function();

    for (size_t i = 0; i < threads.size(); i++)
    {
        threads[i].join();
    }
    
    return;
}

int main(int argc, char *argv[])
{
    int height, common_side, width, threads_count;
    bool print_intermidiate_calculations = false;
    bool print_matrices = false;

    if(!get_values(height, common_side, width, threads_count, argc, argv)) {
        return 1;
    }

    MatrixCellCounter counter{width, height};
    Matrix resulting_matrix{width, height};
    const auto matrices = generate_matrices(height, common_side, width);
    if(print_matrices) {
        std::cout << "Left: \n" << matrices.first << std::endl;
        std::cout << "Right: \n" << matrices.second << std::endl;
    }

    auto worker_function = [&matrices, &counter, &resulting_matrix, print_intermidiate_calculations]() {
        MatrixCell current_cell = counter.get_and_increase();
        while (current_cell.x != -1)
        {
            auto value = calculate_matrix_cell(current_cell, matrices.first, matrices.second);
            if(print_intermidiate_calculations) {
                std::stringstream str;
                str << current_cell << ": " << value << std::endl;
                std::cout << str.str();
            }
            resulting_matrix.set(current_cell, value);
            current_cell = counter.get_and_increase();
        }
        
        return;
    };

    launch_threads_and_wait(threads_count, worker_function);

    if(print_matrices) {
        std::cout << "Resulting matrix: \n" << resulting_matrix;
    }

    return 0;
}
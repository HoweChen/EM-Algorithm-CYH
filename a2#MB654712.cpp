#include <array>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

// string
// std::string fileAddress = "/Users/howechen/ClionProjects/cppTest/input.txt";

// bool
bool STOP = false;

// constant
const int MAX_WORKER = 500; // max number of user
const int MAX_DATA = 2000;  // max number of data from each user
const int MAX_LEVEL = 5;

// array
std::array<std::array<int, MAX_DATA>, MAX_WORKER> input_data;
std::array<std::array<std::array<double, MAX_LEVEL>, MAX_LEVEL>, MAX_WORKER>
    worker_data;
// [-2, -1, 0, 1, 2] = [0, 1, 2, 3, 4] for index in worker_data
std::array<std::array<double, MAX_LEVEL>, MAX_DATA> majority_table;

// vector
std::vector<int> last_iteration;

// function
void
StoreData(std::array<std::array<int, MAX_DATA>, MAX_WORKER> &input_data,
          std::ifstream &InputFile);

void
Initialize_WorkerData(
    std::array<std::array<std::array<double, MAX_LEVEL>, MAX_LEVEL>,
               MAX_WORKER> &worker_data);

void
Initialize_Majority_Table(
    std::array<std::array<int, MAX_DATA>, MAX_WORKER> &input_data,
    std::array<std::array<double, MAX_LEVEL>, MAX_DATA> &majority_table);

void
Pretend_Label_Correct(
    int count_n2,
    int count_n1,
    int count_0,
    int count_1,
    int count_2,
    std::array<std::array<double, MAX_LEVEL>, MAX_DATA> &majority_table,
    int row);

void
Clear_Count(int &count_n2,
            int &count_n1,
            int &count_0,
            int &count_1,
            int &count_2);

void
Recalculate_Worker_Scores(
    std::array<std::array<int, MAX_DATA>, MAX_WORKER> &input_data,
    std::array<std::array<std::array<double, MAX_LEVEL>, MAX_LEVEL>,
               MAX_WORKER> &worker_data,
    std::array<std::array<double, MAX_LEVEL>, MAX_DATA> &majority_table);

int
Level_to_Index(int input);

void
Update_Estimate(
    std::array<std::array<int, MAX_DATA>, MAX_WORKER> &input_data,
    std::array<std::array<double, MAX_LEVEL>, MAX_DATA> &majority_table,
    std::array<std::array<std::array<double, MAX_LEVEL>, MAX_LEVEL>,
               MAX_WORKER> &worker_data);

void
Initialize_First_Iteration(
    std::vector<int> &last_iteration,
    std::array<std::array<double, MAX_LEVEL>, MAX_DATA> majority_table);

void
Check_If_Cover(
    std::vector<int> &last_iteration,
    std::vector<int> &new_iteration,
    std::array<std::array<double, MAX_LEVEL>, MAX_DATA> majority_table);

int
Index_to_Level(int input)
{
    int result = 0;
    switch (input)
    {
        case 0:result = -2;
            break;
        case 1:result = -1;
            break;
        case 2:result = 0;
            break;
        case 3:result = 1;
            break;
        case 4:result = 2;
            break;
        default:break;
    }
    return result;
}
int
main(int argc, char const *argv[])
{
    /* code */
    using namespace std;
    int loop_count = 0;
    std::string fileAddress;
    cout << "Please input the absolute path of your input file" << endl;
    cin >> fileAddress;

    ifstream open_file(fileAddress);
    if (open_file.is_open())
    {
        // file has been opened
        StoreData(input_data, open_file);
        open_file.close();
        //    Initialize_WorkerData(worker_data);
        Initialize_Majority_Table(input_data, majority_table);
        Initialize_First_Iteration(last_iteration, majority_table);

        while (!STOP)
        {
            loop_count += 1;
            vector<int> temp;
            Initialize_WorkerData(worker_data);
            Recalculate_Worker_Scores(input_data, worker_data, majority_table);
            Update_Estimate(input_data, majority_table, worker_data);
            Check_If_Cover(last_iteration, temp, majority_table);
        }
        //    cout << setprecision(2) << fixed << 2.584 << endl;
    }
    else
    {
        cout << "File cannot be opened" << endl;
    }

    // output the result
    ofstream writeFile;
    writeFile.open("./result.txt", ios::trunc);
    writeFile << "Coverage iterations: " << loop_count << endl;
    writeFile << "\nitem_id\t\tTrue Label" << endl;
    for (int i = 0; i < last_iteration.size(); ++i)
    {
        writeFile << i << "\t\t" << last_iteration[i] << endl;
    }

    writeFile << "\n---------------------------\n";
    writeFile << "Worker Accuracy:\n"
              << endl;
    for (int i = 0; i < MAX_WORKER; ++i)
    {
        writeFile << "worker_id:" << i << endl;
        writeFile << "  \t\t-2  \t\t  -1  \t\t  0  \t\t  1  \t\t  2" << endl;
        for (int j = 0; j < MAX_LEVEL; ++j)
        {
            writeFile << Index_to_Level(j);
            for (int k = 0; k < MAX_LEVEL; ++k)
            {
                writeFile << setprecision(2) << fixed << "\t\t" << worker_data[i][j][k];
            }
            writeFile << "\n";
        }
        writeFile << "\n";
    }
    return 0;
}

int
Level_to_Index(int input)
{
    int result = 0;
    switch (input)
    {
        case -2:result = 0;
            break;
        case -1:result = 1;
            break;
        case 0:result = 2;
            break;
        case 1:result = 3;
            break;
        case 2:result = 4;
            break;
        default:break;
    }
    return result;
}

void
Initialize_First_Iteration(
    std::vector<int> &last_iteration,
    std::array<std::array<double, MAX_LEVEL>, MAX_DATA> majority_table)
{
    for (int i = 0; i < MAX_DATA; ++i)
    {
        auto max =
            std::max_element(majority_table[i].begin(), majority_table[i].end());
        long position = std::distance(majority_table[i].begin(), max);
        last_iteration.push_back(Index_to_Level((int) position));
    }
}

void
StoreData(std::array<std::array<int, MAX_DATA>, MAX_WORKER> &input_data,
          std::ifstream &InputFile)
{
    for (int i = 0; i < MAX_WORKER; ++i)
    {
        std::string line;
        if (getline(InputFile, line))
        {
            char *token = strtok((char *) line.c_str(), " ");
            for (int j = 0; j < MAX_DATA; ++j)
            {
                if (token!=nullptr)
                {
                    input_data[i][j] = atoi(token);
                    token = strtok(nullptr, " ");
                }
            }
        }
    }
}

void
Initialize_WorkerData(
    std::array<std::array<std::array<double, MAX_LEVEL>, MAX_LEVEL>,
               MAX_WORKER> &worker_data)
{
    for (int i = 0; i < MAX_WORKER; ++i)
    {
        for (int j = 0; j < MAX_LEVEL; ++j)
        {
            std::fill(worker_data[i][j].begin(), worker_data[i][j].end(), 0);
        }
    }
}

void
Initialize_Majority_Table(
    std::array<std::array<int, MAX_DATA>, MAX_WORKER> &input_data,
    std::array<std::array<double, MAX_LEVEL>, MAX_DATA> &majority_table)
{
    int count_n2 = 0;
    int count_n1 = 0;
    int count_0 = 0;
    int count_1 = 0;
    int count_2 = 0;
    for (int i = 0; i < MAX_DATA; ++i)
    {
        for (int j = 0; j < MAX_WORKER; ++j)
        {
            //      std::cout << input_data[j][i] << std::endl;
            int element = input_data[j][i];
            switch (element)
            {
                case -2:count_n2 += 1;
                    break;
                case -1:count_n1 += 1;
                    break;
                case 0:count_0 += 1;
                    break;
                case 1:count_1 += 1;
                    break;
                case 2:count_2 += 1;
                    break;
                default:std::cout << "Error" << std::endl;
                    //          std::cout << i << "-" << j << std::endl;
                    //          std::cout << input_data[i][j] << std::endl;
                    break;
            }
        }
        Pretend_Label_Correct(count_n2, count_n1, count_0, count_1, count_2,
                              majority_table, i);
        Clear_Count(count_n2, count_n1, count_0, count_1, count_2);
    }
}

void
Pretend_Label_Correct(
    int count_n2,
    int count_n1,
    int count_0,
    int count_1,
    int count_2,
    std::array<std::array<double, MAX_LEVEL>, MAX_DATA> &majority_table,
    int row)
{
    std::vector<int> temp;
    temp.push_back(count_n2);
    temp.push_back(count_n1);
    temp.push_back(count_0);
    temp.push_back(count_1);
    temp.push_back(count_2);
    std::vector<int>::iterator max = std::max_element(temp.begin(), temp.end());
    long position = std::distance(temp.begin(), max);
    majority_table[row][position] = 1;
}

void
Clear_Count(int &count_n2,
            int &count_n1,
            int &count_0,
            int &count_1,
            int &count_2)
{
    count_n2 = 0;
    count_n1 = 0;
    count_0 = 0;
    count_1 = 0;
    count_2 = 0;
}

void
Recalculate_Worker_Scores(
    std::array<std::array<int, MAX_DATA>, MAX_WORKER> &input_data,
    std::array<std::array<std::array<double, MAX_LEVEL>, MAX_LEVEL>,
               MAX_WORKER> &worker_data,
    std::array<std::array<double, MAX_LEVEL>, MAX_DATA> &majority_table)
{
    std::vector<long> temp;
    for (int i = 0; i < MAX_DATA; ++i)
    {
        auto result =
            std::find(majority_table[i].begin(), majority_table[i].end(), 1);
        long position = std::distance(majority_table[i].begin(), result);
        temp.push_back(position);
    }
    for (int i = 0; i < MAX_WORKER; ++i)
    {
        for (int j = 0; j < MAX_DATA; ++j)
        {
            int index = Level_to_Index(input_data[i][j]);
            worker_data[i][temp[j]][index] += 1;
        }
    }

    // count occurrence
    std::vector<std::vector<int>> user_occur;
    double occur_n2 = 0;
    double occur_n1 = 0;
    double occur_0 = 0;
    double occur_1 = 0;
    double occur_2 = 0;
    for (int i = 0; i < MAX_LEVEL; ++i)
    {
        for (int j = 0; j < MAX_DATA; ++j)
        {
            if (majority_table[j][i]==1)
            {
                // count occurrence based on index of array
                switch (i)
                {
                    case 0:occur_n2 += 1;
                        break;
                    case 1:occur_n1 += 1;
                        break;
                    case 2:occur_0 += 1;
                        break;
                    case 3:occur_1 += 1;
                        break;
                    case 4:occur_2 += 1;
                        break;
                    default:break;
                }
            }
        }
    }
    //  double sum = occur_n2 + occur_n1 + occur_0 + occur_1 + occur_2; // sum =
    //  2000

    // update the accuracy of each user
    for (int i = 0; i < MAX_WORKER; ++i)
    {
        for (int j = 0; j < MAX_LEVEL; ++j)
        {
            for (int k = 0; k < MAX_LEVEL; ++k)
            {
                switch (j)
                {
                    case 0:worker_data[i][j][k] = worker_data[i][j][k]/occur_n2;
                        break;
                    case 1:worker_data[i][j][k] = worker_data[i][j][k]/occur_n1;
                        break;
                    case 2:
                        if (occur_0==0)
                        {
                            worker_data[i][j][k] = 0;
                        }
                        else
                        {
                            worker_data[i][j][k] = worker_data[i][j][k]/occur_0;
                        }
                        break;
                    case 3:worker_data[i][j][k] = worker_data[i][j][k]/occur_1;
                        break;
                    case 4:worker_data[i][j][k] = worker_data[i][j][k]/occur_2;
                        break;
                }
            }
        }
    }
    //  std::cout << "test" << std::endl;
}

void
Update_Estimate(
    std::array<std::array<int, MAX_DATA>, MAX_WORKER> &input_data,
    std::array<std::array<double, MAX_LEVEL>, MAX_DATA> &majority_table,
    std::array<std::array<std::array<double, MAX_LEVEL>, MAX_LEVEL>,
               MAX_WORKER> &worker_data)
{
    // clear the majority table¶¶
    for (int i = 0; i < MAX_DATA; ++i)
    {
        std::fill(majority_table[i].begin(), majority_table[i].end(), 0);
    }
    // main part of this function
    for (int i = 0; i < MAX_DATA; ++i)
    {
        for (int j = 0; j < MAX_WORKER; ++j)
        {
            int guess_index = Level_to_Index(input_data[j][i]);
            for (int k = 0; k < MAX_LEVEL; ++k)
            {
                majority_table[i][k] += worker_data[j][k][guess_index];
            }
        }
    }
    //  std::cout << "Test" << std::endl;

    for (int i = 0; i < MAX_DATA; ++i)
    {
        auto max =
            std::max_element(majority_table[i].begin(), majority_table[i].end());
        long position = std::distance(majority_table[i].begin(), max);
        std::fill(majority_table[i].begin(), majority_table[i].end(), 0);
        majority_table[i][position] = 1;
    }
    //  std::cout << "Test" << std::endl;
}

void
Check_If_Cover(
    std::vector<int> &last_iteration,
    std::vector<int> &new_iteration,
    std::array<std::array<double, MAX_LEVEL>, MAX_DATA> majority_table)
{
    for (int i = 0; i < MAX_DATA; ++i)
    {
        auto max =
            std::max_element(majority_table[i].begin(), majority_table[i].end());
        long position = std::distance(majority_table[i].begin(), max);
        new_iteration.push_back(Index_to_Level((int) position));
    }

    // compare with last iteration result
    if (last_iteration==new_iteration)
    {
        STOP = true;
    }

    // store the last result
    last_iteration = new_iteration;
}
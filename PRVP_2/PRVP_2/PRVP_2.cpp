#include <iostream>
#include <mpi.h>
#include <random>

const double EPS = 0.000001;

double FuncOne(double x)
{
    return (1 - exp(0.7 / x)) / (2 + x);
}

double FuncTwo(double x, double y)
{
    return x/(y*y);
}

double fRand(double fMin, double fMax)
{
    double f = (double)rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

void MediumRectangle(int commSize, int commRank)
{
    double a = 1;
    double b = 2;
    double startTime, endTime;
    int n0 = 1;
    int n = n0, k;
    double sq[2], delta = 0;

    startTime = MPI_Wtime();
    for (k = 0; delta < EPS; n *= 2, k ^= 1)
    {
        int pointsPerProc = n / commSize;
        int lb = commRank * pointsPerProc;
        int ub = (commRank == commSize - 1) ? (n - 1) : (lb + pointsPerProc - 1); // разбиениепространства итераций на p смежных непрерывных частей
        double h = (b - a) / n;
        double s = 0.0;

        for (int i = lb; i <= ub; ++i)
            s += FuncOne(a + h * (i + 0.5));
        
        MPI_Allreduce(&s, &sq[k], 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD); // суммирование
        sq[k] *= h;

        if (n > n0)
            delta = fabs(sq[k] - sq[k ^ 1]) / 3.0;
    }
    
    if (commRank == 0)
    {
        endTime = MPI_Wtime();
        std::cout << "Result: " << sq[k] * sq[k] << ", EPS: " << EPS << ", n: " << n / 2;
        std::cout << "\nExecution time " << endTime - startTime << " secounds";
    }
}

void MonteCarlo(int commSize, int commRank, int n)
{
    double startTime, endTime;
    startTime = MPI_Wtime();

    srand(commRank);
    int in = 0;
    double s = 0;
    for (int i = commRank; i < n; i += commSize) 
    {
        double x = fRand(0, 1); 
        double y = fRand(2, 5);
        in++;
        double tmp = FuncTwo(x, y);

        if (tmp == -1)
            continue;
        s += tmp;
    }
    int gin = 0;
    MPI_Reduce(&in, &gin, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    double gsum = 0.0;
    MPI_Reduce(&s, &gsum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (commRank == 0) {
        double v = 1 * gin / n;
        double res = v * gsum / gin;
        endTime = MPI_Wtime();
        std::cout << "Result:" << res << ", n: " << n;
        std::cout << "\nExecution time " << endTime - startTime << " secounds";
    }
}

void main(int argc, char** argv)
{

    //std::cout << "Option 1 paragraph -- " << 6 % 6 + 1; // 1 вариант
    //std::cout << "\nOption 2 paragraph -- " << 6 % 3 + 1; // 1 вариант
    
    MPI_Init(&argc, &argv);
    int commSize, commRank;
    MPI_Comm_size(MPI_COMM_WORLD, &commSize); // функция определяет сколько всего было запущено процессов 
    MPI_Comm_rank(MPI_COMM_WORLD, &commRank); // функция определяет номер каждого процесса

    int optionNumber = atoi(argv[1]);

    if (optionNumber == 1)
        MediumRectangle(commSize, commRank); // 0.02886792
    if (optionNumber == 2)
    {
        int n = atoi(argv[2]); // кол-во вычислений функции
        MonteCarlo(commSize, commRank, n); 
    }
    MPI_Finalize();
}

// mpiexec -n <кол-во процессов> PRVP_2.exe <номер пункта задания> <n (если выбран второй пункт)>
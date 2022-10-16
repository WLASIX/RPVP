#include <mpi.h>
#include <ctime>
#include <iostream>

void InitCharArray(char* array, int arraySize) {
	for (int i = 0; i < arraySize; ++i) {
		array[i] = 65 + rand() % 122;
	}
	return;
}

void Ring(int size, int arraySize, int rank) // кольцо
{
	MPI_Status status;
	double startTime, endTime;
	char* array = (char*)malloc(sizeof(char) * arraySize);
	char* recvArray = (char*)malloc(sizeof(char) * arraySize);

	InitCharArray(array, arraySize);
	startTime = MPI_Wtime();

	MPI_Send(array, arraySize, MPI_CHAR, (rank + 1) % size, 0, MPI_COMM_WORLD);

	std::cout << "Process " << rank << " sent " << (rank + 1) % size << " a message";
	/*for (int i = 0; i < arraySize; ++i)
		cout << array[i];
	cout << endl;*/

	MPI_Recv(recvArray, arraySize, MPI_CHAR, (rank - 1 + size) % size, 0, MPI_COMM_WORLD, &status);

	std::cout << "Process " << rank << " received a message from " << (rank - 1 + size) % size;
	/*for (int i = 0; i < arraySize; ++i)
		cout << recvArray[i];
	cout << endl;*/

	endTime = MPI_Wtime();

	if (rank == 0)
		std::cout << "\nExecution time " << endTime - startTime << " secounds";

	free(array);
	free(recvArray);
	return;
}

void Broadcast(int size, int arraySize, int rank) // трансляционная передача (one-to-all)
{
	MPI_Status status;
	double startTime{}, endTime;
	char* array = (char*)malloc(sizeof(char) * arraySize);
	char* recvArray = (char*)malloc(sizeof(char) * arraySize);

	startTime = MPI_Wtime();

	if (rank == 0)
	{
		InitCharArray(array, arraySize);

		for (int i = 1; i < size; ++i)
		{
			MPI_Send(array, arraySize, MPI_CHAR, i, 0, MPI_COMM_WORLD);
			std::cout << "Process 0 sent " << i << " a message";
			/*for (int j = 0; j < arraySize; ++j)
				cout << array[j];
			cout << endl;*/
		}
	}
	else
	{
		MPI_Recv(recvArray, arraySize, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);

		std::cout << "Process " << rank << " received a message from 0";
		/*for (int i = 0; i < arraySize; ++i)
			cout << recvArray[i];
		cout << endl;*/

		endTime = MPI_Wtime();
		std::cout << "\nExecution time " << endTime - startTime << " secounds";
	}

	free(array);
	free(recvArray);
	return;
}

void Gather(int size, int arraySize, int rank) // коллекторный приём (all-to-one)
{
	MPI_Status status;
	double startTime{}, endTime;
	char* array = (char*)malloc(sizeof(char) * arraySize);
	char* recvArray = (char*)malloc(sizeof(char) * arraySize * size);

	startTime = MPI_Wtime();

	if (rank != 0)
	{
		InitCharArray(array, arraySize);

		MPI_Send(array, arraySize, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
		std::cout << "Process " << rank << " sent 0 a message";
		for (int i = 0; i < arraySize; ++i)
			std::cout << array[i];
		std::cout << std::endl;

	}
	else
	{
		for (int i = 1; i < size; ++i)
		{
			MPI_Recv(recvArray, arraySize, MPI_CHAR, i, 0, MPI_COMM_WORLD, &status);

			std::cout << "Process 0 received a message from " << i << "";
			endTime = MPI_Wtime();
			std::cout << "\nExecution time " << endTime - startTime << " secounds";
		}
	}

	free(array);
	free(recvArray);
	return;
}

void AllToAll(int size, int arraySize, int rank) // каждый процесс передаёт сообщение всем процессам
{
	MPI_Status* status = new MPI_Status[size];
	MPI_Request* sendRequest = new MPI_Request[size];
	MPI_Request* recvRequest = new MPI_Request[size];
	double startTime, endTime;
	char* array = (char*)malloc(sizeof(char) * arraySize);
	char* recvArray = (char*)malloc(sizeof(char) * arraySize * size);

	startTime = MPI_Wtime();

	InitCharArray(array, arraySize);

	for (int i = 0; i < size; ++i)
	{
		MPI_Isend(array, arraySize, MPI_CHAR, i, 0, MPI_COMM_WORLD, sendRequest);
		
		std::cout << "Process " << rank << " sent a message ";
		for (int j = 0; j < arraySize; ++j)
			std::cout << array[j];
		std::cout << std::endl; 
	}
	MPI_Waitall(size, sendRequest, status);

	for (int i = 0; i < size; ++i)
	{
		std::cout << &status[i] << std::endl;
		MPI_Irecv(recvArray + (arraySize * i), arraySize, MPI_CHAR, i, 0, MPI_COMM_WORLD, recvRequest);
		
		
		std::cout << "Process " << rank << " received a message from " << i << " ";
		for (int i = 0; i < arraySize; ++i)
			std::cout << recvArray[i];
		std::cout << std::endl;
	}
	MPI_Waitall(size, recvRequest, status);
	
	endTime = MPI_Wtime();
	std::cout << "Execution time " << endTime - startTime << " secounds";

	free(array);
	free(recvArray);
	return;
	
}

void main(int argc, char** argv)
{
	MPI_Init(&argc, &argv);

	int size, rank;
	int arraySize = atoi(argv[1]);
	int operationNumber = atoi(argv[2]);

	MPI_Comm_size(MPI_COMM_WORLD, &size); // функция определяет сколько всего было запущено процессов 
	MPI_Comm_rank(MPI_COMM_WORLD, &rank); // функция определяет номер каждого процесса

	srand(time(NULL) + rank);

	if (operationNumber == 1) // ring
		Ring(size, arraySize, rank);
	else if (operationNumber == 2) // broadcast
		Broadcast(size, arraySize, rank);
	else if (operationNumber == 3) // gather
		Gather(size, arraySize, rank);
	else if (operationNumber == 4) // all to all
		AllToAll(size, arraySize, rank);

	MPI_Finalize();
}

// mpiexec -n <кол-во процессов> PRVP_1.exe <кол-во символов> <номер операции>

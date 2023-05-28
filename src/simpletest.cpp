#include <iostream>
#include <DataFrame.hpp>
#include <mpi.h>

#define MASTER 0
#define TERMINATE_CODE -1

bool test_serialization() {
	MPI_Init(NULL, NULL);

	int world_size, rank;

	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	printf("Hello world from %d of %d!\n", rank, world_size);

	if (rank == MASTER) {
		DataSlide ds;
		ds.add_param("param1", (int)1);
		ds.add_param("param2", (float)0.5);
		ds.add_data("data1");
		ds.push_data("data1", Sample(1.5, 2, 0.5));

		std::string message = ds.to_string();

		MPI_Send(message.c_str(), message.size(), MPI_CHAR, 1, 0, MPI_COMM_WORLD);
		printf("Master sent message: %s\n", message.c_str());
	} else {
		MPI_Status status;
		MPI_Probe(MASTER, 0, MPI_COMM_WORLD, &status);
		int strlen;
		MPI_Get_count(&status, MPI_CHAR, &strlen);
		printf("Probe found strlen = %d\n", strlen);
		char* buffer = (char*) std::malloc(strlen);
		MPI_Recv(buffer, strlen, MPI_CHAR, MASTER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		std::string ds_data = buffer;
		DataSlide ds = DataSlide::from_string(ds_data);

		printf("Received message: %s\n", ds.to_string().c_str());

		return false;
	}

	MPI_Finalize();

	return true;
}


bool test_pooling() {
    MPI_Init(NULL, NULL);

    int world_size, rank;

    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    std::vector<uint> data;
    uint num_points = 100;
    for (uint i = 0; i < num_points; i++) data.push_back(i);

    std::vector<uint> results(data.size());

    int buffer[2];
    if (rank == MASTER) {
        uint head = 0;
        bool terminate = false;
        std::vector<bool> free(world_size-1, true);
        for (uint j = 0; j < data.size(); j++) {
            //printf("Completed: %d\n", head);
            // Get free process
            for (uint i = 0; i < free.size(); i++) {
                if (free[i]) {
                    free[i] = false;
                    if (terminate){ 
                        buffer[0] = TERMINATE_CODE; buffer[1] = 0;
                    } else {
                        buffer[0] = head; buffer[1] = 0;
                    }

                    //printf("Master sending [%d, %d] to worker %d.\n", buffer[0], buffer[1], i+1);
                    MPI_Send(&buffer, 2, MPI_INT, i+1, 0, MPI_COMM_WORLD);

                    head++;
                    if (head >= data.size())
                        terminate = true;
                }
            }

            // Save result
            MPI_Status status;
            MPI_Recv(&buffer, 2, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            results[buffer[0]] = buffer[1];
            //printf("Master received work from %d.\n", status.MPI_SOURCE-1);
            free[status.MPI_SOURCE-1] = true;
        }

        // Close remaining worker threads
        for (uint i = 0; i < free.size(); i++) {
            if (free[i]) {
                buffer[0] = TERMINATE_CODE; buffer[1] = 0;
            }

            MPI_Send(&buffer, 2, MPI_INT, i+1, 0, MPI_COMM_WORLD);
        }

        std::cout << "From MASTER block: results: \n";
        for (auto s : results) std::cout << s << " ";
        std::cout << "\n";
		return 0;
    } else {
        uint idx;
        while (true) {
            //printf("Worker %d waiting to receive work from master...\n", rank);
            MPI_Recv(&buffer, 2, MPI_INT, MASTER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (buffer[0] == TERMINATE_CODE) {
                //printf("Worker %d received the signal to TERMINATE.\n", rank);
                return 0;
            }
            
            uint idx = buffer[0];
            uint result = buffer[0]*buffer[0];
            buffer[0] = idx; buffer[1] = result;
            //printf("Worker %d sending work to master...\n", rank);
            MPI_Send(&buffer, 2, MPI_INT, MASTER, 0, MPI_COMM_WORLD);
        }
    }

    //std::cout << "Results: \n";
    //for (auto s : results) std::cout << s << " ";
    //std::cout << "\n";

    MPI_Finalize();

    return true;

}

#define DO_IF_MASTER(x) {								\
	int __rank;											\
	MPI_Comm_rank(MPI_COMM_WORLD, &__rank);				\
	if (__rank == MASTER) {								\
		x												\
	}													\
}

int main(int argc, char* argv[]) {
	MPI_Init(NULL, NULL);

	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	DO_IF_MASTER(
		uint i = 15; 
		std::cout << "I AM THE MASTER. i = " << i << "\n";
	)

	MPI_Finalize();
}

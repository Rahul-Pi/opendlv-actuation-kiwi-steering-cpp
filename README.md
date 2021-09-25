# opendlv-actuation-kiwi-steering-cpp
How to get the program running:

################################### Part A: ##############################################

This is to remove any images and containers that can create a conflict with the program.

Caution: This will wipe out all the containers so do it only if it is necessary.

1. Stop all running containers using:     docker stop $(docker ps -a -q)
2. Clear all containers:                  docker rm $(docker ps -a -q)
3. Remove all images:                     docker rmi -f $(docker images -a -q)


################################### Part B: ##############################################

This is to get all the files needed to the local system.

1. Pull the code to your PC/Laptop using
   git clone https://github.com/Rahul-Pi/opendlv-actuation-kiwi-steering-cpp.git

################################### Part C: ##############################################

This is to build the docker from the files.

1. Open a terminal in the folder "opendlv-actuation-kiwi-steering-cpp" and run the following command:
   docker build -f Dockerfile -t opendlv-actuation-kiwi-steering-cpp .


################################### Part D: ##############################################

This part is to run the file and activate the kiwi movement.

1. Open a terminal in the folder "opendlv-actuation-kiwi-steering-cpp" and run the following code to start the actuation of the kiwi. This is to be done after the two perception microservices have been run.
   docker-compose -f simulate-kiwi-actuation.yml up

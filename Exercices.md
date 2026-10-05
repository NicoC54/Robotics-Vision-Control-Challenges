# 🚀 THE ULTIMATE 5 ROBOTICS PROJECTS PORTFOLIO

*From Beginner C++ Architecture to Senior ROS 2 Distributed Systems*

## Level 1: The Multi-Threaded Perception Pipeline

* **Tech Stack:** C++17, OpenCV, `std::thread`, `std::mutex`, Factory/Plugin Pattern.
* **Project Objective:** Build a standalone, high-performance camera perception core that continuously captures frames while concurrently applying a modular, plug-and-play image processing pipeline (e.g., Gaussian Blur followed by Canny Edge Detection). New filters can be added seamlessly without touching core architecture.
* **Engineering Challenge Solved:** Handled `cv::Mat` shallow-copy mechanics to prevent **Data Races** between capture and processing threads, carefully balancing critical sections to maintain high FPS without lock contention.

## Level 2: The Optimized A* Path Planner

* **Tech Stack:** C++ STL, ROS 2 Services, Graph Algorithms.
* **Objective:** Design a ROS 2 Service node (`GetPath`) that accepts an occupancy grid, start pose, and goal pose, computes the optimal collision-free trajectory using the A* algorithm, and returns the sorted waypoint list.
* **Engineering Challenge Solved:** Optimized algorithmic performance by replacing inefficient linear searches with `std::priority_queue`, achieving $O(\log n)$ extraction complexity and avoiding the classic interview trap of using `std::vector` with `std::sort` inside the loop.

## Level 3: The Trajectory Tracking Controller

* **Tech Stack:** ROS 2 Actions, TF2 Transformation Tree, Control Systems (PID).
* **Objective:** Develop a mobile robot Action Server for target tracking ("Follow Object X"). The node queries the TF2 tree to compute relative distances and runs a robust PID controller to generate velocity commands (`cmd_vel`) with real-time feedback and preemption/cancellation handling.
* **Engineering Challenge Solved:** Mitigated **Integral Windup** (preventing error accumulation during actuator stalls) and protected the ROS 2 executor callback queue from blocking loops.

## Level 4: The 3D Pose Estimator

* **Tech Stack:** ROS 2, OpenCV (`solvePnP`), TF2 Broadcaster, Quaternions.
* **Objective:** Implement a computer vision node that detects ArUco markers, computes 3D translation and rotation vectors via camera intrinsics and `solvePnP`, and broadcasts the spatial pose dynamically across the ROS 2 transform network.
* **Engineering Challenge Solved:** Prevented **Gimbal Lock** and rotation ambiguities by strictly avoiding Euler angles (Roll/Pitch/Yaw) in favor of mathematical unit quaternions for TF2 publishing.

## Level 5 (Boss Level): The Zero-Copy State Estimator

* **Tech Stack:** ROS 2 (Compositing, Intra-process Communication, QoS), OpenCV, Linear Kalman Filter.
* **Objective:** Architect two Component Nodes running within a single process. Node 1 publishes camera frames (`SensorData` QoS). Node 2 subscribes via **Zero-Copy** memory sharing, extracts noisy object coordinates, and filters them through a linear Kalman Filter to predict true states and suppress noise.
* **Engineering Challenge Solved:** Managed `toCvShare` memory safety constraints to prevent cross-node memory corruption and resolved complex **QoS profile mismatches** (`Reliable` vs `Best Effort`) that cause silent communication blackouts.

---

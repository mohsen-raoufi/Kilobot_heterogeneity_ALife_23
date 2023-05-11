experiment_name=$1

rm ./src/plugins/robots/kilobot/control_interface/ci_kilobot_controller.cpp
cp ./src/plugins/robots/kilobot/control_interface/ci_kilobot_controller_default.cpp ./src/plugins/robots/kilobot/control_interface/ci_kilobot_controller.cpp


rm ./src/examples/loop_functions/trajectory_loop_functions/trajectory_loop_functions.cpp
cp ./src/examples/loop_functions/trajectory_loop_functions/trajectory_loop_functions_default.cpp ./src/examples/loop_functions/trajectory_loop_functions/trajectory_loop_functions.cpp


rm ./src/examples/experiments/$experiment_name
cp ./src/examples/experiments/defaults/$experiment_name ./src/examples/experiments/$experiment_name

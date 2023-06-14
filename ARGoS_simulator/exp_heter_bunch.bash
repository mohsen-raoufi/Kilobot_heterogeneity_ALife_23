#! /bin/bash

Color_Off='\033[0m'
Blue='\e[033[0;34m'
Green='\033[0;32m'
L_='\033[1;34m'

rm output_*
rm *.config


config_file_name="all_sim.config"

read -p $'\e[33mEnter name of experiment [kilobot_photoTaxis_noVis_randomInit.argos]:\e[0m' experiment_name
experiment_name=${experiment_name:-kilobot_photoTaxis_noVis_randomInit.argos}
echo "experiment_name:" $experiment_name > $config_file_name


read -p $'\e[33mEnter number of repetitions [10]:\e[0m' repetitions
repetitions=${repetitions:-10}
echo "repetitions:" $repetitions >> $config_file_name


read -p $'\e[33mEnter number of robots [100]:\e[0m' number_kilobots
number_kilobots=${number_kilobots:-100}
echo "number_kilobots:" $number_kilobots >> $config_file_name


read -p $'\e[33mEnter heterogeneity factor [0.1]:\e[0m' het_fact
het_fact=${het_fact:-0.1}
echo "het_fact:" $het_fact >> $config_file_name


read -p $'\e[33mEnter noise factor [0.5]:\e[0m' noise_fact
noise_fact=${noise_fact:-0.5}
echo "noise_fact:" $noise_fact >> $config_file_name


read -p $'\e[33mEnter number of time steps to skip for logging [5]:\e[0m' nSkipLog
nSkipLog=${nSkipLog:-5}
echo "n_skip_log:" $nSkipLog >> $config_file_name


read -p $'\e[33mEnter first repetition number to start the simulation from [0]:\e[0m' n_start_reps
n_start_reps=${n_start_reps:-0}
echo "n_start_reps:" $n_start_reps >> $config_file_name


read -p $'\e[33mEnter first id number to start the simulation from [0]:\e[0m' id_to_start
id_to_start=${id_to_start:-0}
echo "n_start_reps:" $id_to_start >> $config_file_name


read -e -p $'\e[33mEnter output folder to move logs to (when all sims are finished):\e[0m' -i "/home/mohsen/heterogeneityStudy/heterogeneitystudy/argos/logFiles/temp" folder_path_to_move_logs
# read -p $'\e[33mEnter output folder to move logs to (when all sims are finished) [/home/mohsen/heterogeneityStudy/heterogeneitystudy/argos/logFiles/temp]:\e[0m' folder_path_to_move_logs
# folder_path_to_move_logs=${folder_path_to_move_logs:-/home/mohsen/heterogeneityStudy/heterogeneitystudy/argos/logFiles/temp}
echo "folder_path_to_move_logs_to:" $folder_path_to_move_logs >> $config_file_name

read -p $'\e[33mRemove the files already in the path you just mentioned? [1]:\e[0m' remove_all_logs
remove_all_logs=${remove_all_logs:-1}
echo "remove_all_logs:" $remove_all_logs >> $config_file_name

read -p $'\e[33mSHUTDOWN WHEN FINISHED? [0]:\e[0m' shut_down_finished
shut_down_finished=${shut_down_finished:-0}

echo -e "${Green} ****************** config info saved into: " $config_file_name " ****************** ${Color_Off}"
echo "--------------------------------------------------------"
echo "------------- READING THE CONFIG FILE ------------------"
cat $config_file_name
echo "--------------------------------------------------------"
echo "--------------------------------------------------------"


sed -i "s/int N = 1/int N = $number_kilobots/" ./src/plugins/robots/kilobot/control_interface/ci_kilobot_controller.cpp
sed -i "s/float heter_factor = 0/float heter_factor = $het_fact/" ./src/plugins/robots/kilobot/control_interface/ci_kilobot_controller.cpp
sed -i "s/m_cNoiseRange.Set(-0.5, 0.5)/m_cNoiseRange.Set(-$noise_fact, $noise_fact)/" ./src/plugins/robots/kilobot/control_interface/ci_kilobot_controller.cpp
sed -i "s/int nSkipLog = 10;/int nSkipLog = $nSkipLog;/" ./src/examples/loop_functions/trajectory_loop_functions/trajectory_loop_functions.cpp
sed -i "s/int arena_width = 1/int arena_width = 4/" ./src/plugins/robots/kilobot/simulator/kilobot_light_rotzonly_sensor.cpp

# update argos according to the number of robots and heter. factor
cd build
sudo make install
cd ..

# Call the other bash file to run each simulation
# For more information of the input args please check the other bash file, before you change anything!
# ./exp_heterogeneity_1Robot_eachTime.bash $experiment_name $number_kilobots 1 0
./exp_heterogeneity_1Robot_multiTrials.bash $experiment_name $number_kilobots $repetitions $n_start_reps $id_to_start

echo -e "${Green} ****************** Experiments Are Finished!! ****************** ${Color_Off}"
echo -e "${Green} **************************************************************** ${Color_Off}"


# Move the new logs to the defined destination folder
if [[ remove_all_logs == 1 ]]; then
  rm -r ${folder_path_to_move_logs}/*
fi
mv output_* $folder_path_to_move_logs
mv *.config $folder_path_to_move_logs

# Organize the destination folder
mkdir ${folder_path_to_move_logs}/logs
mv ${folder_path_to_move_logs}/output*.txt ${folder_path_to_move_logs}/logs
mv ${folder_path_to_move_logs}/output*.config ${folder_path_to_move_logs}/logs

# Copy simulation files to the destination directory
cp ./src/plugins/robots/kilobot/control_interface/ci_kilobot_controller.cpp ${folder_path_to_move_logs}
cp ./src/examples/loop_functions/trajectory_loop_functions/trajectory_loop_functions.cpp ${folder_path_to_move_logs}
cp ./src/plugins/robots/kilobot/simulator/kilobot_light_rotzonly_sensor.cpp ${folder_path_to_move_logs}
cp ./src/examples/experiments/$experiment_name ${folder_path_to_move_logs}
cp -r ./src ${folder_path_to_move_logs}
cp exp_heter_bunch.bash ${folder_path_to_move_logs}
cp exp_heterogeneity_1Robot_multiTrials.bash ${folder_path_to_move_logs}

sed -i "s/int N = $number_kilobots/int N = 1/" ./src/plugins/robots/kilobot/control_interface/ci_kilobot_controller.cpp
sed -i "s/float heter_factor = $het_fact/float heter_factor = 0/" ./src/plugins/robots/kilobot/control_interface/ci_kilobot_controller.cpp
sed -i "s/m_cNoiseRange.Set(-$noise_fact, $noise_fact)/m_cNoiseRange.Set(-0.5, 0.5)/" ./src/plugins/robots/kilobot/control_interface/ci_kilobot_controller.cpp
sed -i "s/int nSkipLog = $nSkipLog;/int nSkipLog = 10;/" ./src/examples/loop_functions/trajectory_loop_functions/trajectory_loop_functions.cpp
sed -i "s/int arena_width = 4/int arena_width = 1/" ./src/plugins/robots/kilobot/simulator/kilobot_light_rotzonly_sensor.cpp


# Replace the argos experiment with the default one
echo -e "${Green} ****************** Replacing the default experiment ****************** ${Color_Off}"

rm ./src/examples/experiments/$experiment_name
cp ./src/examples/experiments/defaults/$experiment_name ./src/examples/experiments/$experiment_name

echo -e "${Green} **************************************************************** ${Color_Off}"
echo -e "${Green} **************************************************************** ${Color_Off}"

if [[ shut_down_finished ]]; then
  sudo systemctl suspend
  # sudo shutdown -h now
fi

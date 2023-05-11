if(ARGOS_BUILD_FOR_SIMULATOR)
  include_directories(${CMAKE_SOURCE_DIR}/plugins/robots/kilobot/control_interface)

  #
  # Lab0: Blinky
  #
  #add_executable(blinky blinky.c)
  # target_link_libraries(blinky argos3plugin_simulator_kilolib)

  #
  # Lab1.2: Simple Movement
  #
  add_executable(simple_movement simple_movement.c)
  target_link_libraries(simple_movement argos3plugin_simulator_kilolib)

  #
  # Lab1.3: Non-blocked Movement
  #
  # add_executable(nonblocked_movement nonblocked_movement.c)
  # target_link_libraries(nonblocked_movement argos3plugin_simulator_kilolib)

  #
  # Lab2.1-2.2: Test Speaker and Test Listener
  #
  # add_executable(test_speaker test_speaker.c)
  # target_link_libraries(test_speaker argos3plugin_simulator_kilolib)
  # add_executable(test_listener test_listener.c)
  # target_link_libraries(test_listener argos3plugin_simulator_kilolib)

  #
  # Lab2.3-2.4: Modified Test Speaker and Test Listener
  #
  # add_executable(test_speaker_mod test_speaker_mod.c)
  # target_link_libraries(test_speaker_mod argos3plugin_simulator_kilolib)
  # add_executable(test_listener_mod test_listener_mod.c)
  # target_link_libraries(test_listener_mod argos3plugin_simulator_kilolib)

  #
  # Lab3: Disperse
  #
  # add_executable(disperse disperse.c)
  # target_link_libraries(disperse argos3plugin_simulator_kilolib)

  #
  # Lab4: Orbit
  #
  # add_executable(orbit_star orbit_star.c)
  # target_link_libraries(orbit_star argos3plugin_simulator_kilolib)
  # add_executable(orbit_planet orbit_planet.c)
  # target_link_libraries(orbit_planet argos3plugin_simulator_kilolib)

  #
  # Lab5: Move to Light
  #
  # add_executable(move_to_light move_to_light.c)
  # target_link_libraries(move_to_light argos3plugin_simulator_kilolib)

  #
  # Lab6: Simple Gradient
  #
  # add_executable(gradient_simple gradient_simple.c)
  # target_link_libraries(gradient_simple argos3plugin_simulator_kilolib)

  #
  # Lab7: Sync
  #
  # add_executable(sync sync.c)
  # target_link_libraries(sync argos3plugin_simulator_kilolib)

  #
  # Debugging example
  #
  # add_executable(test_debug test_debug.h test_debug.c)
  # target_link_libraries(test_debug argos3plugin_simulator_kilolib)

  #
  # Deterministic walk
  #
  add_executable(deterministicWalk deterministicWalk.c)
  target_link_libraries(deterministicWalk argos3plugin_simulator_kilolib)

  #
  # chemotaxis walk
  #
  add_executable(chemotaxis_adaptive_Cond_ver3_correct chemotaxis_adaptive_Cond_ver3_correct.c)
  target_link_libraries(chemotaxis_adaptive_Cond_ver3_correct argos3plugin_simulator_kilolib)

  #
  # Chemotaxis with different chance to left/right (maybe 50-50)
  #
  add_executable(chemotaxis_adaptive_Cond_ver3_correct_LoR chemotaxis_adaptive_Cond_ver3_correct_LoR.c)
  target_link_libraries(chemotaxis_adaptive_Cond_ver3_correct_LoR argos3plugin_simulator_kilolib)


  #
  # random walk unconditional: random_walk
  #
  add_executable(random_walk random_walk.c)
  target_link_libraries(random_walk argos3plugin_simulator_kilolib)

  #
  # Our dispersion method: can be conditional or unconditional on signal strength
  #
  add_executable(dispersion dispersion.c)
  target_link_libraries(dispersion argos3plugin_simulator_kilolib)

  # Standard Contour Capturing
  add_executable(contour_capturing contour_capturing.c)
  target_link_libraries(contour_capturing argos3plugin_simulator_kilolib)

  # Contour Capturing for changing it during the bunch run
  add_executable(contour_capturing_forChanging contour_capturing_forChanging.c)
  target_link_libraries(contour_capturing_forChanging argos3plugin_simulator_kilolib)

endif(ARGOS_BUILD_FOR_SIMULATOR)

idf_component_register(  
     INCLUDE_DIRS "aaa"
     SRCS "aaa/aaa.c"
     
     INCLUDE_DIRS "app"
     SRCS "app/app.cpp" 
          "app/console.cpp" 
          "app/MqttConnection.cpp" 
          "app/task_list.cpp" 
          "app/task_console.cpp" 
          "app/task_cloud.cpp" 
          "app/task_device.cpp" 
          "app/task_ota.cpp" 
          "app/task_ui.cpp"
          "app/task_net.cpp"
     
     INCLUDE_DIRS "common"
     SRCS "common/cmd_line.c"
          "common/utils.c"
     
     INCLUDE_DIRS "driv"
     SRCS "driv/button.c"
     
     INCLUDE_DIRS "sys"
     SRCS "sys/sys_io.c"
          "sys/sys_log.c"
)

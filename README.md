# arduino_mazebot
Robot based on the mBot (Arduino) platform designed to traverse a maze.

![MeetTheMbot](https://i.imgur.com/ZiZ6meh.png)

## Maze Challenges
  - Collision detection
    Using IR sensors and a front Ultrasonic sensor, the bot was able to detect its proximity to walls by polling the voltage returned by these sensors.
    
    ![CollisionDetection](https://i.imgur.com/Um5tJrl.png)
  - Colour detection
    The bot employed LDRs to detect any colours above it. The maze was designed such that each colour corresponds to a certain action - which was hardcoded into the bot.
    
    ![ColourTurning](https://i.imgur.com/WzJmFLp.jpg)
  - Sound detection
    The hardest challenge involved setting up a circuit with filters to compare the amplitude of 300Hz and 3000Hz frequencies that were playing from a speaker simultaneously. Based on which frequency's amplitude was higher, the bot would perform a predefined action.
    
    ![Breadboard](https://i.imgur.com/7PQ74v9.jpg)

# Cages
Rat cages code

This project currently has two different options.
The newly added code goes over the power saving mode. This is a description of how that works:

<h2> Power Saving Mode Instructions: </h2>

1. There are two seperate "pauses" in delays between the delays that are used to conserve power
 - The first one is how much time before the cages are supposed to go off that the delays stop working
 - This is currently set to 10 minutes, this variable can be changed in the `clienthandler.h` file under the variable `stop_delay_before_cages`
 - The second delay is in between each delay. Currently it is set to 5 minutes. Basically in between each massive delay, there is a small delay of 5 minutes. This is there so in case the main Head needs to connect and give it new alarms or other commands.
 - This variable is currently by default set to 5 minutes, in the `clienthandler.h` file under the variable `time_between_delays`


 2. There is a different setup to the webpage
 - This new power saving mode lists all the different cages and their IP addresses at the bottom of the webpage
 - <b>DO NOT IGNORE THIS, THIS IS IMPORTANT</b>
 - If you unplug the main webserver for what ever reason, and do not want to unplug/disconnect/reset the cages, you MUST document the current IP addresses of the cages.
 - The reason being, you can manually enter in the IP addresses of the new cages into the webpages.
 - You will have to do this because there are very high chances that the cages will be in their designated "power saving mode" when you plug the webserver back in and try to find the nodes again.
 - These are the steps for unplugging the webserver then plugging it back in:
     1. Write down all of the current cages' IP addresses that are shown at the bottom of the page.
          - otherwise you will have to unplug then re-plug all the cages that you want to work back in.
      2. Plug the webserver back in.
      3. Before you write down all the new IP addresses, make sure that you MUST correct the current time of the webserver if you want it to work propperly.</ul>
      4. Then enter in the IP-addresses of the 
 - Other than that, everything pretty much works the same, however, once you press the "send alarms" button, (which is new) the power saving mode will start, and this will result in the loss of the function of the open, close, opposite buttons.

<br/><p>
Further testing needs to be done on the power save mode to specifically determe how well it is working and what needs to be updated/changed</p>

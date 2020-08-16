# eMoments
DISCLAIMER: This project requires a BlueTooth connection to an authenticated machine at G. (i.e. only FTEs at G. have access)

## Description
eMoments is a 20% project which allows FTEs to voice their opinion about common topics. For example, after an #allhands meeting, attendees can write a few words of text (think shorter Tweets) and select one of 5 colors to express their mood. There's an internal web page which displays each entry as a block of color with text and any hash tags assigned. In sum, eMoments gives both audience members a voice, and business leaders immediate feedback about their initiatives.

## Hackathon Competition
Currently the eMoments (20%) team are running an internal contest. The idea is to socialize the project and get the data out of our screens and onto our desktops to heighten awareness. Below are Liz's contest entries.

### NeoTrellis Grid
<img src = "https://github.com/LizMyers/eMoments/blob/master/emoments_arduino_neotrellis.ino/neotrellis.jpg" width="40%" height = "auto" alt = "photo of neo trellis" />
The idea for this implementation came from an earlier project: <a href="https://learn.adafruit.com/neotrellis-soundboard/">Neotrellis Sound Board</a>. The NeoTrellis works well because each of the 16 LEDs is invidividually addressable and can be assigned independent actions on button press. In this design, each LED represents a unique eMoment. **By pushing the button in one of the four columns, users can  change the display** according to 1 of 4 pre-built patterns: Render by Color, Render by Time, Render by One Color, Render Randomly.

### DotStar Crawler
<img src = "https://github.com/LizMyers/eMoments/blob/master/emoments_dotstar_crawler/dotstar.PNG" width = "40%" height="auto" alt="photo of dot star crawler" />
The DotStar Matrix provides a 6 x 12 grid or 72 individually addressable pixels. However, assigning each pixel an individual eMoment proved too dense and fatigued the eyes very quickly. So Liz came up with a better plan. This little matrix is much better at displaying simple icons and text. So Liz decided to build the word Goooooooooooooogle out of eMoments data and scroll the multi-colored text across the screen. 

To add a bit of interactivity, there's an accelerometer inside. **A single tap on the case toggles between 2 display modes:** Render by Time and Render by One Color (the most recent eMoment or the first char in colorString.)

<br>
[LM TO-DO: Add 3D printed case and swap photos here]

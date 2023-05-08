  static const char PROGMEM INDEX_HTML[] = R"rawliteral(
  <html>
  <head>
    <title>Cam</title> <!--  tab title -->
    <meta name="viewport" content="width=device-width, initial-scale=1">

    <style>       <!-- styling section. This sections sepfices what each element will look like.-->
      body {
        font-family: Arial;
        text-align: center;
        margin: 0px auto;
        padding-top: 30px;
      }

      table {
        margin-left: auto;
        margin-right: auto;
      }

      td {
        padding: 8 px;
      }

      .button {
        background-color: black;
        color: white;
        padding: 10px 20px;
        text-align: center;
        display: inline-block;
        font-size: 18px;
        margin: 0px 0px;
        cursor: pointer;
      }

      img {
        width: auto;
        max-width: 100%;
        height: auto;
      }

    </style>
  </head>
  <body>
    <h1>CamBot</h1> <!-- body of the html. This is what the user will see -->
    <img src="" id="photo">
    <table>
      <tr>
        <td colspan="3" align="center"> <!-- top button for foward. a on button press, e on button release -->
          <button class="button" onmousedown="sendData('a');" ontouchstart="sendData('a');" onmouseup="sendData('e');" ontouchend="sendData('e');">Forward</button>
        </td>
      </tr>
      <tr>
        <td align="center">           <!--  button for left. d on button press, e on button release -->
          <button class="button" onmousedown="sendData('d');" ontouchstart="sendData('d');" onmouseup="sendData('e');" ontouchend="sendData('e');">Left</button>
        </td>
        <td align="center">           <!-- stop button for foward. r on button press -->
          <button class="button" onmousedown="sendData('e');" ontouchstart="sendData('e');">Stop</button>
        </td>
        <td align="center">         <!--  button for right. c on button press, e on button release -->
          <button class="button" onmousedown="sendData('c');" ontouchstart="sendData('c');" onmouseup="sendData('e');" ontouchend="sendData('e');">Right</button>
        </td>
      </tr>
      <tr>
        <td colspan="3" align="center"> <!-- bottom button for reverse. b on button press, e on button release -->
          <button class="button" onmousedown="sendData('b');" ontouchstart="sendData('b');" onmouseup="sendData('e');" ontouchend="sendData('e');">Backward</button>
        </td>
      </tr>
      <tr>
        <td colspan="3" align="center"> <!-- label, button, and text box for the song. Button calls sendSong() -->
          <label for="Song">Song:</label>
          <input type="text" id="Song" name="Song" placeholder="0">
          <button class="button" onmousedown="sendSong();" ontouchstart="sendSong();">Send</button> 
        </td>
      </tr>
      <tr>
        <td colspan="3" align="center">   <!-- label, button, and text box for the speed. Button calls sendSpeed() -->
          <label for="Song">Speed:</label>
          <input type="text" id="Speed" name="Speed" placeholder="0">
          <button class="button" onmousedown="sendSpeed();" ontouchstart="sendSpeed();">Send</button>
        </td>
      </tr>
    </table>


    <script> <!-- JavaScript section -->

    function convertintChar(integer) { <!-- function to convert value to character, so only one byte needs to be sent -->

         let character = 'a'.charCodeAt(0);
         return String.fromCharCode(character + integer);
    }

      function sendData(x) { <!-- function to send data based on whats passed in -->
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "/command?go=" + x, true);    <!-- sends a get request with the input as the payload. uses the /command tag. -->
        xhr.send();
      }

      function sendSpeed() { <!-- function to send data regarding the speed input -->
        var inputVal = document.getElementById("Speed").value;     <!-- gets textbox value-->
        var c = convertintChar(inputVal);                          <!-- textbox value to char-->
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "/command?go=" + '0' + c, true);           <!-- sends a get request with the char 0 and song vlaue as the payload. uses the /command tag. --> 
        xhr.send();
      }

      function sendSong() <!-- function to send data regarding the song input -->
        var inputVal = document.getElementById("Song").value;
        var c = convertintChar(inputVal);
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "/command?go=" + '1' + c, true);
        xhr.send();
      }
      window.onload = document.getElementById("photo").src = window.location.href.slice(0, -1) + ":81/video";
      <!-- this window load is responsible for loading the stream. It is not a video, rather a ton of images being sent over every request -->
    </script>
  </body>
</html> 
  )rawliteral";

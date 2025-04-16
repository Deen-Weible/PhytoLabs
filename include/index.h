const char MAIN_page[] PROGMEM = R"=====(<!DOCTYPE html>
<html lang="en">

<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Document</title>
  <style>
    /* Existing styles remain unchanged */
    .card-container {
      display: flex;
      justify-content: center;
      min-height: 100vh;
    }

    .card {
      border: 2px solid #ccc;
      height: min-content;
      width: 100%;
      max-width: 600px;
      margin: 20px auto;
      display: flex;
      flex-direction: column;
      justify-content: center;
      padding: 25px;
    }

    .input-group {
      margin-bottom: 15px;
    }

    .form-input {
      width: 100%;
      padding: 8px;
      border: 1px solid #ccc;
      margin-right: 10px;
      border-radius: 4px;
    }

    .pair {
      display: flex;
      justify-content: space-between;
      margin-bottom: 15px;
    }

    button {
      background-color: #313131;
      color: white;
      padding: 10px 20px;
      border: none;
      cursor: pointer;
      width: 100%;
      box-shadow: 3px 4px 4px rgba(0, 0, 0, 0.3);
      transition: all 0.3s ease;
    }

    button:hover {
      background-color: #222222;
      transform: translateY(-2px);
      scale: 1.01;
    }

    /* New styles for the file input button */
    #file-upload::file-selector-button {
      background-color: #313131;
      color: white;
      padding: 8px 16px;
      border: none;
      border-radius: 4px;
      cursor: pointer;
      margin-right: 10px;
      transition: background-color 0.3s ease;
    }

    #file-upload::file-selector-button:hover {
      background-color: #222222;
    }
  </style>

<body>
  <div class="card-container">
    <div class="card">
      <div class="input-group">
        <div class="pair">
          <input type="number" name="hour" id="hour" placeholder="hour" class="form-input">
          <input type="number" name="minute" id="minute" placeholder="minute" class="form-input">
        </div>
      </div>
      <div class="input-group">
        <div class="pair">
          <input type="number" name="num2" id="num2" placeholder="Number 2" class="form-input">
          <input type="number" name="num3" id="num3" placeholder="Number 3" class="form-input">
        </div>
        <div class="pair" style="border: 1px solid #ccc;;">
          <input type="file" id="file-upload" accept=".bin" class="form-input" style="border:none">
          <button onclick="sendFile()" style="margin:8px">Update</button>
        </div>
      </div>
      <button onclick="SendForms()">Send Forms</button>
    </div>
  </div>
  <div id="ADCValue">bingus</div>
</body>

<script>
  function sendFile() {
    const xhr = new XMLHttpRequest();

    xhr.open('POST', '/update-firmware');
    xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
    xhr.upload.addEventListener('progress', function (evt) {
      if (evt.lengthComputable) {
        const per = evt.loaded / evt.total;
      }
    });
    xhr.send(document.getElementById("file-upload").files[0]);

  }
  function getData() {
    var xhttp = new XMLHttpRequest();
    xhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");

    xhttp.onreadystatechange = function () {
      if (this.readyState == 4 && this.status == 200) {
        document.getElementById("ADCValue").innerHTML = this.responseText;
      }
    };
    xhttp.open("GET", "readADC", true);
    xhttp.send();
  }

  function sendData(url, data, callback, method = "POST") {
    var xhttp = new XMLHttpRequest();
    xhttp.open(method, url, true);
    xhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");

    // Prepare parameters (assuming we're sending one parameter named "param1")
    // Optional logging
    console.log("Sending: ", data);

    xhttp.onreadystatechange = function () {
      if (xhttp.readyState === 4 && this.status == 400) {
        // You can adjust the status check or callback as necessary.
        if (typeof callback === "function") {
          callback(xhttp);
        }
      }
    };
    xhttp.send(data);
    return xhttp;
  }

  var obj = [];
  var xhttp = new XMLHttpRequest();
  // obj.num = document.getElementById("num1").value
  // obj.str = document.getElementById("str1").value

  function SendForms() {
    var inputs = document.querySelectorAll('.form-input');
    var numberDictionary = "";

    var obj = new Object();

    obj.Hour = document.getElementById('hour').value;
    obj.Minute = document.getElementById('minute').value;

    numberDictionary = `${JSON.stringify(obj)}`;

    for (let i = 0; i < inputs.length; i++) {
      const input = inputs[i];
      const key = input.name;
    }

    console.log(numberDictionary);
    var response = sendData("SendForms", numberDictionary)
    document.getElementById("ADCValue").innerHTML = numberDictionary;
    // Adjust status check as needed (e.g., checking for 200 or other statuses)
    response.onreadystatechange = function () {
      if (response.status === 200) {
        console.log("Request succeeded:", response.responseText);
        // document.getElementById("ADCValue").innerHTML = response.responseText;
      } else {
        console.log("Request failed with status", response.status);
        // document.getElementById("ADCValue").innerHTML = response.responseText;
      }
    }
  }
</script>
</body>

</html>)=====";
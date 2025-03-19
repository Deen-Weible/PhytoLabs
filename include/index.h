const char MAIN_page[] PROGMEM = R"=====(<!DOCTYPE html>
<html lang="en">

<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Document</title>
  <style>
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
      /* min-height: 80vh; Ensures card is at least visible on the page */
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
      /* Aligns inputs horizontally */
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
      /* Smooth hover effects */
    }

    button:hover {
      background-color: #222222;
      transform: translateY(-2px);
      scale: 1.01;
    }

    button:active {
      animation: pressAnimation 0.1s ease-out forwards;
    }

    /* Keyframes for the press animation */
    @keyframes pressAnimation {
      0% {
        /* transform: translateY(0) scale(1); */
        box-shadow: 3px 4px 4px rgba(0, 0, 0, 0.3);
      }

      50% {
        transform: translateY(2px) scale(0.99);
        box-shadow: 1px 2px 2px rgba(0, 0, 0, 0.2);
      }

      100% {
        /* transform: translateY(0) scale(1); */
        box-shadow: 3px 4px 4px rgba(0, 0, 0, 0.3);
      }
    }

    #ADCValue {
      background-color: #434343;
      color: white;
      padding: 15px;
      border-radius: 4px;
      margin-bottom: 20px;
      position: absolute;
      right: 15px;
      bottom: 15px;
      width: min(85vw, 550px);
    }

    @media (max-width: 700px) {
      #ADCValue {
        /* top: auto; */
        width:90vw;
        bottom:15px;
        left: 0;
        right: 0;
        margin-left: auto;
        margin-right: auto;
      }
    }
  </style>
</head>

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
      </div>
      <button onclick="SendForms()">Send Forms</button>
    </div>
  </div>
  <div id="ADCValue">bingus</div>
</body>

<script>
  function getData() {
    var xhttp = new XMLHttpRequest();
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

    // for (let i = 0; i < inputs.length; i++) {
    //   const input = inputs[i];
    //   const key = input.name; // Using i+1 because the first input is num1
    //   numberDictionary += ("&" + key + "=" + encodeURIComponent(input.value));
    //   console.log("Value: " + input.value);
    // }

    var obj = new Object();

    obj.Hour = document.getElementById('hour').value;
    obj.Minute = document.getElementById('minute').value;

    numberDictionary = `${JSON.stringify(obj)}`;

    for (let i = 0; i <inputs.length; i++) {
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
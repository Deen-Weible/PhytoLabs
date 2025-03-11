const char MAIN_page[] PROGMEM = R"=====(<html>

<head>
	<title>Document</title>
</head>

<body>

	<div class="card">
		<input type="number" name="num1" id="num1" placeholder="num">
		<input type="text" name="str1" id="str1" placeholder="text">
		<button onclick="SendForms()">Send ze' forms</button>
		<div id="ADCValue"></div>
	</div>

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
			var params = `param1=${encodeURIComponent(JSON.stringify(data))}`;

			// Optional logging
			console.log("Sending: ", params);

			xhttp.onreadystatechange = function () {
				if (xhttp.readyState === 4 && this.status == 400) {
					// You can adjust the status check or callback as necessary.
					if (typeof callback === "function") {
						callback(xhttp);
					}
				}
			};
			xhttp.send(params);
			return xhttp;
		}

		var obj = new Object();
		var xhttp = new XMLHttpRequest();
		// obj.num = document.getElementById("num1").value
		// obj.str = document.getElementById("str1").value

		function SendForms() {
			var response = sendData("SendForms", document.getElementById("num1").value)
				// Adjust status check as needed (e.g., checking for 200 or other statuses)
				response.onreadystatechange = function () {
				if (response.status === 200) {
					console.log("Request succeeded:", response.responseText);
				} else {
					console.log("Request failed with status", response.status);
					document.getElementById("ADCValue").innerHTML = response.responseText;
				}
			}}
	</script>
</body>

</html>)=====";
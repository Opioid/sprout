window.onload = function() {
	var consoleForm = document.getElementById('consoleForm');
	var consoleInput = document.getElementById('consoleInput');

	var viewerImage = document.getElementById('viewerImage');

	viewerImage.width  = 1;
	viewerImage.height = 1;

	var context = viewerImage.getContext("2d");
	var imageData = context.getImageData(0, 0, 1, 1); // only do this once per page
	var target  = imageData.data; 

	// Create a new WebSocket.
	var socket = new WebSocket('ws://localhost:8080');
	socket.binaryType = "arraybuffer";

	// Handle any errors that occur.
	socket.onerror = function(error) {
		console.log('WebSocket Error: ' + error);
	};

	// Show a connected message when the WebSocket is opened.
	socket.onopen = function(event) {
		// socketStatus.innerHTML = 'Connected to: ' + event.currentTarget.URL;
		// socketStatus.className = 'open';
	};

	// Handle messages sent by the server.
	socket.onmessage = function(event) {
		if (event.data instanceof Blob) {
			/*
			viewerImage.src = URL.createObjectURL(event.data);
			viewerImage.onload = function() {
				URL.revokeObjectURL(this.src);
			}*/


		//	myContext.putImageData( id, x, y );

		} else if (event.data instanceof ArrayBuffer) {
			/*
			var binaryData = new Uint8Array(event.data);
			var blob = new Blob([binaryData], { type: 'image/png' }); 
			viewerImage.src = URL.createObjectURL(blob);
			viewerImage.onload = function() {
				URL.revokeObjectURL(this.src);
			}
			*/

			var binaryData = new Uint8Array(event.data);

			target.set(binaryData);
			context.putImageData(imageData, 0, 0);
		} else {

			// var thing = JSON.parse(event.data, function(k, v) {
			// 	if (k === 'resolution') {
			// 		console.log(v);
			// 	//	console.log(v[1]);
			// 	}
			// });

			var messageObject = JSON.parse(event.data);

			var resolution = messageObject["resolution"];

			viewerImage.width  = resolution[0];
			viewerImage.height = resolution[1];

			context.fillRect(0, 0, resolution[0], resolution[1]);

			imageData = context.getImageData(0, 0, resolution[0], resolution[1]);
			target = imageData.data;

			consoleInput.style['width'] = (resolution[0] - 4) + 'px';
		}
	};

	// Show a disconnected message when the WebSocket is closed.
	socket.onclose = function(event) {
		// socketStatus.innerHTML = 'Disconnected from WebSocket.';
		// socketStatus.className = 'closed';
	};

	// Send a message when the form is submitted.
	consoleForm.onsubmit = function(e) {
		e.preventDefault();

		// Retrieve the message from the textarea.
		var message = consoleInput.value;

    	// Send the message through the WebSocket.
		socket.send(message);

		// Clear out the message field.
		consoleInput.value = '';

		return false;
	};
};
 

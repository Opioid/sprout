window.onload = function() {
	var viewerImage = document.getElementById('viewerImage');

	// Create a new WebSocket.
	var socket = new WebSocket('ws://localhost:8080');
//	socket.binaryType = "arraybuffer";

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
			viewerImage.src = URL.createObjectURL(event.data);
			viewerImage.onload = function() {
				URL.revokeObjectURL(this.src);
			}
		} else if (event.data instanceof ArrayBuffer) {
			var binaryData = new Uint8Array(event.data);
			var blob = new Blob([binaryData], { type: 'image/png' }); 
			viewerImage.src = URL.createObjectURL(blob);
			viewerImage.onload = function() {
				URL.revokeObjectURL(this.src);
			}
		} else {
			// var message = event.data;
			// messagesList.innerHTML += '<li class="received"><span>Received:</span>' 
			// 						+ message + '</li>';
		}
	};

	// Show a disconnected message when the WebSocket is closed.
	socket.onclose = function(event) {
		// socketStatus.innerHTML = 'Disconnected from WebSocket.';
		// socketStatus.className = 'closed';
	};
};
 

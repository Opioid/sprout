window.onload = function() {
	var viewerImage = document.getElementById('viewerImage');

	// Create a new WebSocket.
	var socket = new WebSocket('ws://127.0.0.1:8080');

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
			var urlCreator = window.URL || window.webkitURL;
			var imageUrl = urlCreator.createObjectURL(event.data);
			viewerImage.src = imageUrl;
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
 

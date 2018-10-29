IterationCounter = function() {
    this.count = 0;
    this.label = document.getElementById("iterationLabel");
};

IterationCounter.prototype.constructor = IterationCounter;

IterationCounter.prototype.init = function(iteration) {
    this.count = iteration;
    this.display();
};

IterationCounter.prototype.update = function() {
    ++this.count;
    this.display();
};

IterationCounter.prototype.display = function() {
    this.label.innerHTML = "# " + this.count; 
};

FpsCounter = function() {
    this.previousTime = 0.0;
    this.frameTimeCounter = 0.0;
    this.frameCounter = 0;
    this.label = document.getElementById("fpsLabel");
};

FpsCounter.prototype.constructor = FpsCounter;

FpsCounter.prototype.update = function() {
    let currentTime = performance.now();
    let frameTime = (currentTime - this.previousTime) / 1000.0;

    this.previousTime = currentTime;

    this.frameTimeCounter += frameTime;

    this.frameCounter++;

    if (this.frameTimeCounter > 1.0) {
        let fps = Math.floor((this.frameCounter / this.frameTimeCounter) * 10.0) / 10.0;
        this.label.innerHTML = fps + " FPS";

        this.frameTimeCounter = 0.0;
        this.frameCounter = 0;
	}
};

window.onload = function() {
    let consoleForm = document.getElementById("consoleForm");
    let consoleInput = document.getElementById("consoleInput");

    let viewerImage = document.getElementById("viewerImage");

    viewerImage.width  = 1;
    viewerImage.height = 1;

    viewerImage.addEventListener("click", function(event) {
        if (this === document.pointerLockElement) {
            document.exitPointerLock();
        } else {
            this.requestPointerLock();
        }
    });

    let mouseAxis = [0.0, 0.0, 0.0];
	
    viewerImage.addEventListener("mousemove", function(event) {
        if (this === document.pointerLockElement) {
		    mouseAxis[0] += event.movementX;
	        mouseAxis[1] += event.movementY;
        }		
    });

    viewerImage.addEventListener("wheel", function(event) {
        if (this === document.pointerLockElement) {
            mouseAxis[2] += event.deltaY;
        }
    });

    let context = viewerImage.getContext("2d");
    let imageData = context.getImageData(0, 0, 1, 1); // only do this once per page
    let target  = imageData.data; 

    let iterationCounter = new IterationCounter();
    let fpsCounter = new FpsCounter();

    // Create a new WebSocket.
    let socket = new WebSocket("ws://localhost:8080");
    socket.binaryType = "arraybuffer";

    // Handle any errors that occur.
    socket.onerror = function(error) {
        console.log("WebSocket Error: " + error);
    };

    // Show a connected message when the WebSocket is opened.
    socket.onopen = function(event) {
        // socketStatus.innerHTML = 'Connected to: ' + event.currentTarget.URL;
        // socketStatus.className = 'open';
	};

    // Handle messages sent by the server.
    socket.onmessage = function(event) {
        if (event.data instanceof Blob) {
			// 
        } else if (event.data instanceof ArrayBuffer) {
            iterationCounter.update();
            fpsCounter.update();

            let binaryData = new Uint8Array(event.data);

            target.set(binaryData);
            context.putImageData(imageData, 0, 0);

            // Send back accumulated mouse axis delta
            if (0.0 !== mouseAxis[0] || 0.0 !== mouseAxis[1] || 0.0 !== mouseAxis[2]) {
                socket.send("md:[" + mouseAxis + "]");

                mouseAxis[0] = 0.0;
                mouseAxis[1] = 0.0;
                mouseAxis[2] = 0.0;
            }
        } else {
            let messageObject = JSON.parse(event.data);

            let resolution = messageObject["resolution"];
            if (undefined !== resolution) {
                viewerImage.width  = resolution[0];
                viewerImage.height = resolution[1];

                context.fillRect(0, 0, resolution[0], resolution[1]);

                imageData = context.getImageData(0, 0, resolution[0], resolution[1]);
                target = imageData.data;

                consoleInput.style["width"] = (resolution[0] - 4) + "px";
            }

            let iteration = messageObject["iteration"];
            if (undefined !== iteration) {
                iterationCounter.init(iteration);
            }
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
        let message = consoleInput.value;

        // Send the message through the WebSocket.
        socket.send(message);

        // Clear out the message field.
        consoleInput.value = "";

        return false;
    };
};


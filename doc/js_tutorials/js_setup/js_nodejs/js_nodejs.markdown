Using OpenCV.js In Node.js {#tutorial_js_nodejs}
==========================

Goals
-----

In this tutorial, you will learn:

-   Use OpenCV.js in a [Node.js](https://nodejs.org) application.
-   Load images with [jimp](https://www.npmjs.com/package/jimp) in order to use them with OpenCV.js.
-   Using [jsdom](https://www.npmjs.com/package/canvas) and [node-canvas](https://www.npmjs.com/package/canvas) to support `cv.imread()`, `cv.imshow()`
-   The basics of [emscripten](https://emscripten.org/) APIs, like [Module](https://emscripten.org/docs/api_reference/module.html) and [File System](https://emscripten.org/docs/api_reference/Filesystem-API.html) on which OpenCV.js is based.
-   Learn Node.js basics. Although this tutorial assumes the user knows JavaScript, experience with Node.js is not required.

@note Besides giving instructions to run OpenCV.js in Node.js, another objective of this tutorial is to introduce users to the basics of [emscripten](https://emscripten.org/) APIs, like [Module](https://emscripten.org/docs/api_reference/module.html) and [File System](https://emscripten.org/docs/api_reference/Filesystem-API.html) and also Node.js.


Minimal example
-----

Create a file `example1.js` with the following content:

@code{.js}
// Define a global variable 'Module' with a method 'onRuntimeInitialized':
Module = {
  onRuntimeInitialized() {
    // this is our application:
    console.log(cv.getBuildInformation())
  }
}
// Load 'opencv.js' assigning the value to the global variable 'cv'
cv = require('./opencv.js')
@endcode

### Execute it ###

-   Save the file as `example1.js`.
-   Make sure the file `opencv.js` is in the same folder.
-   Make sure [Node.js](https://nodejs.org) is installed on your system.

The following command should print OpenCV build information:

@code{.bash}
node example1.js
@endcode

### What just happened? ###

 * **In the first statement**:, by defining a global variable named 'Module', emscripten will call `Module.onRuntimeInitialized()` when the library is ready to use. Our program is in that method and uses the global variable `cv` just like in the browser.
 * The statement **"cv = require('./opencv.js')"** requires the file `opencv.js` and assign the return value to the global variable `cv`.
   `require()` which is a Node.js API, is used to load modules and files.
   In this case we load the file `opencv.js` form the current folder, and, as said previously emscripten will call `Module.onRuntimeInitialized()` when its ready.
 * See [emscripten Module API](https://emscripten.org/docs/api_reference/module.html) for more details.


Working with images
-----

OpenCV.js doesn't support image formats so we can't load png or jpeg images directly. In the browser it uses the HTML DOM (like HTMLCanvasElement and HTMLImageElement to decode and decode images). In node.js we will need to use a library for this.

In this example we use [jimp](https://www.npmjs.com/package/jimp), which supports common image formats and is pretty easy to use.

### Example setup ###

Execute the following commands to create a new node.js package and install [jimp](https://www.npmjs.com/package/jimp) dependency:

@code{.bash}
mkdir project1
cd project1
npm init -y
npm install jimp
@endcode

### The example ###

@code{.js}
const Jimp = require('jimp');

async function onRuntimeInitialized(){

  // load local image file with jimp. It supports jpg, png, bmp, tiff and gif:
  var jimpSrc = await Jimp.read('./lena.jpg');

  // `jimpImage.bitmap` property has the decoded ImageData that we can use to create a cv:Mat
  var src = cv.matFromImageData(jimpSrc.bitmap);

  // following lines is copy&paste of opencv.js dilate tutorial:
  let dst = new cv.Mat();
  let M = cv.Mat.ones(5, 5, cv.CV_8U);
  let anchor = new cv.Point(-1, -1);
  cv.dilate(src, dst, M, anchor, 1, cv.BORDER_CONSTANT, cv.morphologyDefaultBorderValue());

  // Now that we are finish, we want to write `dst` to file `output.png`. For this we create a `Jimp`
  // image which accepts the image data as a [`Buffer`](https://nodejs.org/docs/latest-v10.x/api/buffer.html).
  // `write('output.png')` will write it to disk and Jimp infers the output format from given file name:
  new Jimp({
    width: dst.cols,
    height: dst.rows,
    data: Buffer.from(dst.data)
  })
  .write('output.png');

  src.delete();
  dst.delete();
}

// Finally, load the open.js as before. The function `onRuntimeInitialized` contains our program.
Module = {
  onRuntimeInitialized
};
cv = require('./opencv.js');
@endcode

### Execute it ###

-   Save the file as `exampleNodeJimp.js`.
-   Make sure a sample image `lena.jpg` exists in the current directory.

The following command should generate the file `output.png`:

@code{.bash}
node exampleNodeJimp.js
@endcode


Emulating HTML DOM and canvas
-----

As you might already seen, the rest of the examples use functions like `cv.imread()`, `cv.imshow()` to read and write images. Unfortunately as mentioned they won't work on Node.js since there is no HTML DOM.

In this section, you will learn how to use [jsdom](https://www.npmjs.com/package/canvas) and [node-canvas](https://www.npmjs.com/package/canvas)  to emulate the HTML DOM on Node.js so those functions work.

### Example setup ###

As before, we create a Node.js project and install the dependencies we need:

@code{.bash}
mkdir project2
cd project2
npm init -y
npm install canvas jsdom
@endcode

### The example ###

@code{.js}
const { Canvas, createCanvas, Image, ImageData, loadImage } = require('canvas');
const { JSDOM } = require('jsdom');
const { writeFileSync, existsSync, mkdirSync } = require("fs");

// This is our program. This time we use JavaScript async / await and promises to handle asynchronicity.
(async () => {

  // before loading opencv.js we emulate a minimal HTML DOM. See the function declaration below.
  installDOM();

  await loadOpenCV();

  // using node-canvas, we an image file to an object compatible with HTML DOM Image and therefore with cv.imread()
  const image = await loadImage('./lena.jpg');

  const src = cv.imread(image);
  const dst = new cv.Mat();
  const M = cv.Mat.ones(5, 5, cv.CV_8U);
  const anchor = new cv.Point(-1, -1);
  cv.dilate(src, dst, M, anchor, 1, cv.BORDER_CONSTANT, cv.morphologyDefaultBorderValue());

  // we create an object compatible HTMLCanvasElement
  const canvas = createCanvas(300, 300);
  cv.imshow(canvas, dst);
  writeFileSync('output.jpg', canvas.toBuffer('image/jpeg'));
  src.delete();
  dst.delete();
})();

// Load opencv.js just like before but using Promise instead of callbacks:
function loadOpenCV() {
  return new Promise(resolve => {
    global.Module = {
      onRuntimeInitialized: resolve
    };
    global.cv = require('./opencv.js');
  });
}

// Using jsdom and node-canvas we define some global variables to emulate HTML DOM.
// Although a complete emulation can be archived, here we only define those globals used
// by cv.imread() and cv.imshow().
function installDOM() {
  const dom = new JSDOM();
  global.document = dom.window.document;

  // The rest enables DOM image and canvas and is provided by node-canvas
  global.Image = Image;
  global.HTMLCanvasElement = Canvas;
  global.ImageData = ImageData;
  global.HTMLImageElement = Image;
}
@endcode

### Execute it ###

-   Save the file as `exampleNodeCanvas.js`.
-   Make sure a sample image `lena.jpg` exists in the current directory.

The following command should generate the file `output.jpg`:

@code{.bash}
node exampleNodeCanvas.js
@endcode


Dealing with files
-----

In this tutorial you will learn how to configure emscripten so it uses the local filesystem for file operations instead of using memory. Also it tries to describe how [files are supported by emscripten applications](https://emscripten.org/docs/api_reference/Filesystem-API.html)

Accessing the emscripten filesystem is often needed in OpenCV applications for example to load machine learning models such as the ones used in @ref tutorial_dnn_googlenet and @ref tutorial_dnn_javascript.

### Example setup ###

Before the example, is worth consider first how files are handled in emscripten applications such as OpenCV.js. Remember that OpenCV library is written in C++ and the file opencv.js is just that C++ code being translated to JavaScript or WebAssembly by emscripten C++ compiler.

These C++ sources use standard APIs to access the filesystem and the implementation often ends up in system calls that read a file in the hard drive. Since JavaScript applications in the browser don't have access to the local filesystem, [emscripten emulates a standard filesystem](https://emscripten.org/docs/api_reference/Filesystem-API.html) so compiled C++ code works out of the box.

In the browser, this filesystem is emulated in memory while in Node.js there's also the possibility of using the local filesystem directly. This is often preferable since there's no need of copy file's content in memory. This section explains how to do just that, this is, configuring emscripten so files are accessed directly from our local filesystem and relative paths match files relative to the current local directory as expected.

### The example ###

@code{.js}
const { Canvas, createCanvas, Image, ImageData, loadImage } = require('canvas');
const { JSDOM } = require('jsdom');
const { writeFileSync, existsSync, mkdirSync } = require('fs');
const https = require('https');

(async () => {
const createFileFromUrl = function (path, url, maxRedirects = 10) {
  console.log('Downloading ' + url + '...');
  return new Promise((resolve, reject) => {
    const download = (url, redirectCount) => {
      if (redirectCount > maxRedirects) {
        reject(new Error('Too many redirects'));
      } else {
        let connection = https.get(url, (response) => {
          if (response.statusCode === 200) {
            let data = [];
            response.on('data', (chunk) => {
              data.push(chunk);
            });

            response.on('end', () => {
              try {
                writeFileSync(path, Buffer.concat(data));
                resolve();
              } catch (err) {
                reject(new Error('Failed to write file ' + path));
              }
            });
          } else if (response.statusCode === 302 || response.statusCode === 301) {
            connection.abort();
            download(response.headers.location, redirectCount + 1);
          } else {
            reject(new Error('Failed to load ' + url + ' status: ' + response.statusCode));
          }
        }).on('error', (err) => {
          reject(new Error('Network Error: ' + err.message));
        });
      }
    };
    download(url, 0);
  });
};

if (!existsSync('./face_detection_yunet_2023mar.onnx')) {
  await createFileFromUrl('./face_detection_yunet_2023mar.onnx', 'https://media.githubusercontent.com/media/opencv/opencv_zoo/main/models/face_detection_yunet/face_detection_yunet_2023mar.onnx')
}

if (!existsSync('./opencv.js')) {
  await createFileFromUrl('./opencv.js', 'https://docs.opencv.org/5.x/opencv.js')
}

if (!existsSync('./lena.jpg')) {
  await createFileFromUrl('./lena.jpg', 'https://docs.opencv.org/5.x/lena.jpg')
}

await loadOpenCV();

const image = await loadImage('./lena.jpg');
const src = cv.imread(image);
let srcBGR = new cv.Mat();
cv.cvtColor(src, srcBGR, cv.COLOR_RGBA2BGR);

// Load the deep learning model file. Notice how we reference local files using relative paths just
// like we normally would do
let netDet = new cv.FaceDetectorYN("./face_detection_yunet_2023mar.onnx", "", new cv.Size(320, 320), 0.9, 0.3, 5000);
netDet.setInputSize(new cv.Size(src.cols, src.rows));
let out = new cv.Mat();
netDet.detect(srcBGR, out);

let faces = [];
for (let i = 0, n = out.data32F.length; i < n; i += 15) {
  let left = out.data32F[i];
  let top = out.data32F[i + 1];
  let right = (out.data32F[i] + out.data32F[i + 2]);
  let bottom = (out.data32F[i + 1] + out.data32F[i + 3]);
  left = Math.min(Math.max(0, left), src.cols - 1);
  top = Math.min(Math.max(0, top), src.rows - 1);
  right = Math.min(Math.max(0, right), src.cols - 1);
  bottom = Math.min(Math.max(0, bottom), src.rows - 1);

  if (left < right && top < bottom) {
    faces.push({
      x: left,
      y: top,
      width: right - left,
      height: bottom - top,
      x1: out.data32F[i + 4] < 0 || out.data32F[i + 4] > src.cols - 1 ? -1 : out.data32F[i + 4],
      y1: out.data32F[i + 5] < 0 || out.data32F[i + 5] > src.rows - 1 ? -1 : out.data32F[i + 5],
      x2: out.data32F[i + 6] < 0 || out.data32F[i + 6] > src.cols - 1 ? -1 : out.data32F[i + 6],
      y2: out.data32F[i + 7] < 0 || out.data32F[i + 7] > src.rows - 1 ? -1 : out.data32F[i + 7],
      x3: out.data32F[i + 8] < 0 || out.data32F[i + 8] > src.cols - 1 ? -1 : out.data32F[i + 8],
      y3: out.data32F[i + 9] < 0 || out.data32F[i + 9] > src.rows - 1 ? -1 : out.data32F[i + 9],
      x4: out.data32F[i + 10] < 0 || out.data32F[i + 10] > src.cols - 1 ? -1 : out.data32F[i + 10],
      y4: out.data32F[i + 11] < 0 || out.data32F[i + 11] > src.rows - 1 ? -1 : out.data32F[i + 11],
      x5: out.data32F[i + 12] < 0 || out.data32F[i + 12] > src.cols - 1 ? -1 : out.data32F[i + 12],
      y5: out.data32F[i + 13] < 0 || out.data32F[i + 13] > src.rows - 1 ? -1 : out.data32F[i + 13],
      confidence: out.data32F[i + 14]
    })
  }
}
out.delete();

faces.forEach(function(rect) {
  cv.rectangle(src, {x: rect.x, y: rect.y}, {x: rect.x + rect.width, y: rect.y + rect.height}, [0, 255, 0, 255]);
  if(rect.x1>0 && rect.y1>0)
    cv.circle(src, {x: rect.x1, y: rect.y1}, 2, [255, 0, 0, 255], 2)
  if(rect.x2>0 && rect.y2>0)
    cv.circle(src, {x: rect.x2, y: rect.y2}, 2, [0, 0, 255, 255], 2)
  if(rect.x3>0 && rect.y3>0)
    cv.circle(src, {x: rect.x3, y: rect.y3}, 2, [0, 255, 0, 255], 2)
  if(rect.x4>0 && rect.y4>0)
    cv.circle(src, {x: rect.x4, y: rect.y4}, 2, [255, 0, 255, 255], 2)
  if(rect.x5>0 && rect.y5>0)
    cv.circle(src, {x: rect.x5, y: rect.y5}, 2, [0, 255, 255, 255], 2)
});

const canvas = createCanvas(image.width, image.height);
cv.imshow(canvas, src);
writeFileSync('output3.jpg', canvas.toBuffer('image/jpeg'));
console.log('The result is saved.')
src.delete(); srcBGR.delete();
})();

/**
 * Loads opencv.js.
 *
 * Installs HTML Canvas emulation to support `cv.imread()` and `cv.imshow`
 *
 * Mounts given local folder `localRootDir` in emscripten filesystem folder `rootDir`. By default it will mount the local current directory in emscripten `/work` directory. This means that `/work/foo.txt` will be resolved to the local file `./foo.txt`
 * @param {string} rootDir The directory in emscripten filesystem in which the local filesystem will be mount.
 * @param {string} localRootDir The local directory to mount in emscripten filesystem.
 * @returns {Promise} resolved when the library is ready to use.
 */
function loadOpenCV(rootDir = '/work', localRootDir = process.cwd()) {
  if(global.Module && global.Module.onRuntimeInitialized && global.cv && global.cv.imread) {
   Promise.resolve()
  }
  return new Promise(resolve => {
    installDOM()
    global.Module = {
      onRuntimeInitialized() {
        // We change emscripten current work directory to 'rootDir' so relative paths are resolved
        // relative to the current local folder, as expected
        cv.FS.chdir(rootDir)
        resolve()
      },
      preRun() {
        // preRun() is another callback like onRuntimeInitialized() but is called just before the
        // library code runs. Here we mount a local folder in emscripten filesystem and we want to
        // do this before the library is executed so the filesystem is accessible from the start
        const FS = global.Module.FS
        // create rootDir if it doesn't exists
        if(!FS.analyzePath(rootDir).exists) {
          FS.mkdir(rootDir);
        }
        // create localRootFolder if it doesn't exists
        if(!existsSync(localRootDir)) {
          mkdirSync(localRootDir, { recursive: true});
        }
        // FS.mount() is similar to Linux/POSIX mount operation. It basically mounts an external
        // filesystem with given format, in given current filesystem directory.
        FS.mount(FS.filesystems.NODEFS, { root: localRootDir}, rootDir);
      }
    };
    global.cv = require('./opencv.js')
  });
}

function installDOM(){
  const dom = new JSDOM();
  global.document = dom.window.document;
  global.Image = Image;
  global.HTMLCanvasElement = Canvas;
  global.ImageData = ImageData;
  global.HTMLImageElement = Image;
}
@endcode

### Execute it ###

-   Save the file as `exampleNodeCanvasData.js`.
-   The files `face_detection_yunet_2023mar.onnx`, `lena.jpg` and `opencv.js` will be downloaded if they not present in project's directory.

The following command should generate the file `output3.jpg` look the image below:

@code{.bash}
node exampleNodeCanvasData.js
@endcode

![image](js_assets/lena_yunet.jpg)

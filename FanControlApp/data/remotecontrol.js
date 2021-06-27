// For development purpose
const HostIP = "192.168.0.160";

function isRunLocalFile() {
    switch (window.location.protocol) {
        case 'http:':
        case 'https:':
            return false;
            break;

        case 'file:':
            return true;
            break;

        default:
            return true;
    }
}

function sendControl(code, len, protocol, pulselen) {
    var param = "code=" + code;
    if (len) {
        param += "&len=" + len;
    }
    if (protocol) {
        param += "&protocol=" + protocol;
    }
    if (pulselen) {
        param += "&pulselen=" + pulselen;
    }
    console.log(param);

    var xhr = new XMLHttpRequest();
    xhr.open("GET", (isRunLocalFile() ? ("http://" + HostIP) : "") + "/rf?" + param);
    xhr.send();
}
var imageFile = document.querySelector('#imageFile');
var oldFileList ="";

imageFile.onchange = function() {
  var preview = document.querySelector('#preview');
  var button = document.querySelector('#upload-button');
  var resultJson = document.getElementById("post-result");

  var reader = new FileReader();
  reader.onload = function() {
    if (resultJson.hasChildNodes()) {
      resultJson.removeChild(resultJson.firstChild);
    }
    preview.src = reader.result;
    button.disabled = false;
    preview.onload = function() {
      var canvas = document.querySelector('#rectCanvas');
      canvas.width = preview.width;
      canvas.height = preview.height;
    }
  }

  var fileList = imageFile.files;
  if (fileList && fileList[0]) {
    reader.readAsDataURL(fileList[0]);
    oldFileList = fileList;
  } else {
    imageFile.files = oldFileList;
  }
}

function drawRectOnImage(json) {
  var canvas = document.querySelector('#rectCanvas');
  var canvasContext = canvas.getContext('2d');
  canvasContext.strokeStyle = 'rgba(255, 0, 0, 0.7)';
  canvasContext.clearRect(0, 0, canvas.width, canvas.height);

  if (json.faces) {
    json.faces.forEach(value => {
      canvasContext.strokeRect(value.x, value.y, value.width, value.height);
    });
  }
}

$(document).ready(function() {
  $('.button-submit').on('click', function(e){
    var id = $(this).attr('data-formid');
    $('#' +  id).submit();
  });

  $('#form-image-upload').submit(function(e) {
    e.preventDefault();
    var formData = new FormData(this);
    var url = $(this).attr('action');
    $('.button-submit').attr("disabled", true);
    $.ajax({
      url: url,
      type: 'POST',
      data: formData,
      success: function(data) {
        drawRectOnImage(data);
        $('#post-result').html('<pre>' +
          JSON.stringify(data, undefined, 2) + '</pre>');
          $('.button-submit').attr("disabled", false);
      },
      error: function(request,status,error) {
        $('#post-result').html("code: " +
          request.status+", message: " + request.responseText);
        $('.button-submit').attr("disabled", false);
      },
      cache: false,
      contentType: false,
      processData: false
    });
  });
});

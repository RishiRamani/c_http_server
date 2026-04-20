const output = document.getElementById("output");

function show(data) {
  output.textContent = data;
}

// POST
function sendPost() {
  const value = document.getElementById("postInput").value;

  fetch("/submit", {
    method: "POST",
    body: value
  })
    .then(res => res.text())
    .then(show)
    .catch(err => show(err));
}

// GET all
function getAll() {
  fetch("/data")
    .then(res => res.text())
    .then(show)
    .catch(err => show(err));
}

// GET by ID
function getById() {
  const id = document.getElementById("getId").value;

  fetch(`/data?id=${id}`)
    .then(res => res.text())
    .then(show)
    .catch(err => show(err));
}

// PUT
function updateData() {
  const id = document.getElementById("putId").value;
  const data = document.getElementById("putData").value;

  fetch(`/data?id=${id}`, {
    method: "PUT",
    body: data
  })
    .then(res => res.text())
    .then(show)
    .catch(err => show(err));
}

// DELETE
function deleteData() {
  const id = document.getElementById("deleteId").value;

  let url = "/data";
  if (id) {
    url = `/data?id=${id}`;
  }

  fetch(url, {
    method: "DELETE"
  })
    .then(res => res.text())
    .then(show)
    .catch(err => show(err));
}

function send_automatic_to_server(is_auto){
     const server_location="http://127.0.0.1:5000/"
     fetch(server_location +'home?type=1&state='+ (is_auto ? "1":"0"))
         .then(response => response.text())
         .then(data => {
             if (is_auto)
                alert("Lockdown is not automatic")
             else{
                 alert("Lockdown is automatic")
             }
        }).catch(err => {
            alert(err)
     })
}

function send_lockdown_to_server(is_lock){
     const server_location="http://127.0.0.1:5000/"
     fetch(server_location +'home?type=2&state='+ (is_lock ? "1":"0"))
         .then(response => response.text())
         .then(data => {
            if (is_lock)
                alert("Lockdown is enabled")
             else{
                 alert("Lockdown is disabled")
             }
        }).catch(err => {
            alert(err)
     })
}

function init_home() {
    let locktoggleButton = document.getElementById('lockdownToggle');

    locktoggleButton.addEventListener('click', function () {
         this.checked = !this.checked
        locktoggleButton.classList.toggle('on');
        send_lockdown_to_server(this.checked);
    });

    let autotoggleButton = document.getElementById('automaticToggle');

    // autotoggleButton.addEventListener('click', function() {
    //   autotoggleButton.classList.toggle('on');
    // });

    autotoggleButton.addEventListener('click', function () {
        this.checked = !this.checked
        autotoggleButton.classList.toggle('on');
        if (this.checked) {
            let lock_div = document.getElementById("lockdown_container")
            lock_div.style.display = 'none';
            send_automatic_to_server(this.checked);
        } else {
            let lock_div = document.getElementById("lockdown_container")
            lock_div.style.display = 'flex';
            send_automatic_to_server(this.checked);
        }
    });
}


function begin(){
  init_home();
}

begin();
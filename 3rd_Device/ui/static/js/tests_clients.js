function create_chart_table(chart_id, labels, values, colors, title) {
    // Retrieve the data passed from Flask
    //  var labels = ['Red', 'Blue', 'Yellow'];
    //  var values = [10, 20, 30];
     console.log(labels);
     console.log(values);
     console.log(colors);
     console.log(title)
     // Create the chart
     var ctx = document.getElementById(chart_id).getContext('2d');
     var myChart = new Chart(ctx, {
         type: 'line',
         data: {
             labels: labels,
             datasets: [{
                 fill: false,
                 lineTension: 0,
                 data: values,
                 backgroundColor: colors[0],
                 borderColor: colors[1]
                 // backgroundColor: ['red', 'blue', 'yellow'],
             }]
         },
         options: {
            title: {
              display: true,
              text: title,
                fontColor: "rgb(192,190,190)"
            },
             legend: {display: false},
          },
         scales: {
              yAxes: [{
                ticks: {
                  fontColor: "rgba(192,190,190,1)",
                    min: 0,
                 max:Math.max(labels) +1
                },
                  gridLines: {
                      color: 'rgba(208, 208, 208,0.2)'
                    }

              }],
              xAxes: [{
                ticks: {
                  fontColor: "rgba(192,190,190,1)",
                    min: 0,
                 max:Math.max(values) +1
                },
                  gridLines: {
                      color: 'rgba(208, 208, 208, 0.2)'
                    }
              }]
            }
     })
}


function load_chart_test_scans_stats(ip,mac){
    fetch(server_location +'test_clients?ip='+ip +"&mac=" +mac + "&type=1")
         .then(response => response.text())
         .then(data => {
             console.log(data)
             console.log(JSON.parse(data))
             let labels =[]
             let values =[]
             let colors = ['black', 'red'];
             let title = "Number of scans per day"
             let list_stats = JSON.parse(data)
             //list_stats.sort(function(a, b){return b.value-a.value});
             list_stats= list_stats.reverse()
             let poz = 0;
             for( el of list_stats){
                 labels.push(el.day_of_test);
                 values.push(el.nr_tests);
                 poz++;
                 if (poz >= 10){
                     break;
                 }
             }
             labels = labels.reverse()
             values = values.reverse()
             create_chart_table("canvas_test_scans",labels,values,colors,title);
        }).catch(err => {
            alert("Cannot retrieve categories! Error :" + err);
            // document.getElementById('content').textContent = err;
     })
}

function load_chart_test_malware_stats(ip,mac){
    fetch(server_location +'test_clients?ip='+ip +"&mac=" +mac + "&type=2")
         .then(response => response.text())
         .then(data => {
             console.log(data)
             console.log(JSON.parse(data))
             let labels = []
             let values = []
             let colors = ['black', 'blue'];
             let title = "Number of malware programs detected per day"
             let list_stats = JSON.parse(data)
             //list_stats.sort(function(a, b){return b.value-a.value});
             list_stats= list_stats.reverse()
             let poz = 0;
             for( el of list_stats){
                 labels.push(el.day_of_test);
                 values.push(el.nr_malwares);
                 poz++;
                 if (poz >= 10){
                     break;
                 }
             }
             labels = labels.reverse()
             values = values.reverse()
             create_chart_table("canvas_test_malware",labels,values,colors,title);
        }).catch(err => {
            alert("Cannot retrieve categories! Error :" + err);
            // document.getElementById('content').textContent = err;
     })
}

function display_test_details(test_details){
    console.log(test_details)
    test_details = JSON.parse(test_details)
    if (test_details === {})
        return;
    let test_details_container = document.getElementById("test_details");
    test_details_container.innerHTML = ""
    let title = document.createElement("h2");
    title.innerHTML = "Client Details";
    let list_details = document.createElement("ul");
    let list_keys = [["ID:",test_details.id], ["Client IP:",test_details.ip], ["Status:",test_details.status], ["Time Stated:",test_details.time_start], ["Time Finished:",test_details.time_finished], ["Results:",test_details.results]]
    console.log(list_keys)
    let index_key = 0
    for (let i = 0;i < list_keys.length;i++) {
        let li_det = document.createElement("li");
        let span_det = document.createElement("span");
        span_det.className = "list_extra_details";
        let key_det = document.createElement("p");
        key_det.className = "key";
        key_det.innerHTML = list_keys[index_key][0]
        let value_det = undefined
        console.log(list_keys.length)
        if (index_key !== list_keys.length - 1) {
            value_det = document.createElement("p");
            value_det.innerHTML = list_keys[index_key][1]
        }
        else{ //for results
            console.log(list_keys[index_key][1],Object.keys(list_keys[index_key][1]).length)
            if (Object.keys(list_keys[index_key][1]).length > 0) {
                value_det = document.createElement("ul");
                for (let mal in list_keys[index_key][1]){
                    let count = list_keys[index_key][1][mal];
                    console.log(mal);
                    let mal_li = document.createElement("li");
                    let mal_data = document.createElement("p");

                    mal_data.innerHTML = mal;
                    if (count > 1){
                        mal_data.innerHTML += "  x(" + count + ")";
                    }
                    mal_li.appendChild(mal_data);
                    value_det.appendChild(mal_li);
                }
            }
            else{
                value_det = document.createElement("p");
                value_det.innerHTML = "No malware detected"
            }
        }
        if (value_det) {
            span_det.appendChild(key_det);
            span_det.appendChild(value_det);
            li_det.appendChild(span_det)
            list_details.appendChild(li_det);
        }
        //console.log(li_det)
        index_key+=1;
    }
    test_details_container.appendChild(title);
    test_details_container.appendChild(list_details);
}

function get_test_details(test_id){
     fetch(server_location +'tests?id='+test_id)
         .then(response => response.text())
         .then(data => {
             console.log(data)
             //let div_content= document.getElementById('test_details');
             // div_content.innerHTML = data;
             display_test_details(data);
        }).catch(err => {
            document.getElementById('test_details').textContent = err;
     })
}

function load_data(ip,mac){
    console.log("ip:", ip, "mac:",mac);
    // load_tabel_platforms();
    // load_tabel_categories();
    load_chart_test_scans_stats(ip,mac);
    load_chart_test_malware_stats(ip,mac);
}

function begin(){
    let dropout_container = document.getElementsByClassName("dropout")[0];
    det = dropout_container.id;
    ip = det.split("|")[0]
    mac = det.split("|")[1]
    load_data(ip,mac);
    let list_but_see_details=document.querySelectorAll(".see_details_but");
    console.log("begin", list_but_see_details);
    list_but_see_details.forEach(function(item) {
        item.addEventListener("click",function (){
            console.log("click get test details");
           tr_item = this.parentNode.parentNode;
           id_test = tr_item.getAttribute("id");
           get_test_details(id_test);
        })
      });
}
begin()

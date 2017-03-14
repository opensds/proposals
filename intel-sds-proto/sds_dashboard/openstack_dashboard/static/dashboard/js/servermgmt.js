function post_data(){
    var rows_num = $(".modal-body .name").length - 1;
    var data_list = new Array();
    for (var i=1; i <= rows_num; i++)
    {
        row_id = $($(".modal-body .server_id")[i]).html();
        var row = $("#serversaction__row__" + row_id);
        checked = row.find(".multi_select_column").find("input").is(":checked");
        if(checked == true)
        {
            id = row.find(".multi_select_column").find("input").val();
            zone_id = row.find(".zone").html();
            monitor = row.find(".monitor").find("input").attr("checked") ? true : false;
            storage = row.find(".storage").find("input").attr("checked") ? true : false;

            data = {id:id, is_monitor:monitor, is_storage:storage, zone_id:zone_id};
		    data_list.push(data);
		    resp=JSON.stringify(data_list);
        }

     }
	token=$("input[name=csrfmiddlewaretoken]").val();

	horizon.ajax.queue({
        data: resp,
        type: "post",
        dataType: "json",
        url: "/horizon/admin/provisioning/servers/add",
        success: function (data) {horizon.alert(data.status, data.data);
        var refresh_data = function(){
            $(".status_up").removeClass("status_up").addClass("status_unknown");
            horizon.datatables.update();
            }
        setTimeout(refresh_data, 8000);
        },
        error: function (XMLHttpRequest, textStatus, errorThrown) {
            horizon.alert("error", "Add servers Error");
            horizon.modals.spinner.modal('hide');
        },
        headers: {
          "X-CSRFToken": token
        },
        complete: function(){
          horizon.modals.spinner.modal('hide');
          $(".close").click();
       }
    });

    window.location.reload()						
};

function remove_servers(){
    var rows_num = $(".modal-body .name").length - 1;
    var data_list = new Array();
    for (var i=1; i <= rows_num; i++)
    {
        row_id = $($(".modal-body .server_id")[i]).html();
        var row = $("#serversaction__row__" + row_id);
        checked = row.find(".multi_select_column").find("input").is(":checked");
        if(checked == true)
        {
            id = row.find(".multi_select_column").find("input").val();
            remove_storage = row.find(".remove_storage").find("input").attr("checked") ? true : false;

            if(row.find(".role").html() == "storage,monitor")
            {
                remove_monitor = true;
            }
            else
            {
                remove_monitor = false;
            }

            data = {id:id, remove_monitor:remove_monitor, remove_storage:remove_storage};
		    data_list.push(data);
		    resp=JSON.stringify(data_list);
        }

     }
	token=$("input[name=csrfmiddlewaretoken]").val();

	horizon.ajax.queue({
        data: resp,
        type: "post",
        dataType: "json",
        url: "/horizon/admin/provisioning/servers/remove",
        success: function (data) {horizon.alert(data.status, data.data);
        var refresh_data = function(){
            $(".status_up").removeClass("status_up").addClass("status_unknown");
            horizon.datatables.update();
         }
        setTimeout(refresh_data, 8000);
        },
        error: function (XMLHttpRequest, textStatus, errorThrown) {
            horizon.alert("error", "Add servers Error");
            horizon.modals.spinner.modal('hide');
        },
        headers: {
          "X-CSRFToken": token
        },
        complete: function(){
          horizon.modals.spinner.modal('hide');
          $(".close").click();
       }
    });

    window.location.reload()						
};

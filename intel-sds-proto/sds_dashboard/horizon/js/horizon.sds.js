horizon.addInitFunction(function () {
  /* Launch instance workflow */

  // Handle field toggles for the Launch Instance source type field

  function update_launch_system_displayed_fields (field) {
    var $this = $(field),
      base_type = $this.val();

    $this.closest(".form-group").nextAll().hide();

    switch(base_type) {
      case "system_type":
        $("#id_system_type").closest(".form-group").show();
        break;

      case "ceph":
        $("#id_ceph").closest(".form-group").show();
        break;

      case "swift":
        $("#id_swift").closest(".form-group").show();
        break;

    }
  }

  function update_discover_system_input_fields (field) {
    var $this = $(field),
    base_type = $this.val();

    $this.closest(".form-group").nextAll().hide();

    switch(base_type) {
      case "storage_system":
        $("#id_storage_system").closest(".form-group").show();
        break;

      case "ceph":
        $("#id_ip").closest(".form-group").show();
        $("#id_user").closest(".form-group").show();
        $("#id_fsid").closest(".form-group").show();
        break;

      case "cephx":
        $("#id_ip").closest(".form-group").show();
        $("#id_user").closest(".form-group").show();
        break;

      case "swift":
        $("#id_ip").closest(".form-group").show();
        $("#id_user").closest(".form-group").show();
        $("#id_key").closest(".form-group").show();
        break;

      case "swift_key":
        $("#id_ip").closest(".form-group").show();
        break;

    }
  }

  $(document).on('change', '#id_system_type', function (evt) {
    update_launch_system_displayed_fields(this);
  });

  $(document).on('change', '#id_storage_system', function (evt) {
    update_discover_system_input_fields(this);
  });
  
  $('#id_system_type').change();
  $('#id_storage_system').change();

  horizon.modals.addModalInitFunction(function (modal) {
    $(modal).find("#id_system_type").change();
    $(modal).find("#id_storage_system").change();
  });

});

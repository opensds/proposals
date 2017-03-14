=============================
SDS Dashboard
=============================

SDS Dashboard is a project aimed at providing UI for SDS controller based on
OpenStack Dashboard. It provides a set of patches which allow users to apply.
For now, it is based on Horizon module of Juno OpenStack.

Getting Started
===============
1. Copy sds_dashboard folder to Horizon folder, e.g., /opt/stack/horizon.
$ cd sds-prototype/sds_dashboard
Change enable_sds_dashboard.sh with right HORIZON_PATH and run the script to patch horizon:
$ sudo enable_sds_dashboard.sh

2. Add following entry to /etc/openstack-dashboard/local_settings.py
KEYSTONE_VSM_SERVICE_PASSWORD = <VSM PASSWORD>

3. Go to horizon directory (e.g. /opt/stack/horizon) and run compress
python manage.py compress

4. Restart apache2 service
service apache2 restart

5. After patches are applied, refresh OpenStack Dashboard, a new panel group named
'SDS Controller' will show under 'Admin'.

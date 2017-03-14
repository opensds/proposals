#!/bin/bash
#---------------------------------------------
# Important Variables.
#---------------------------------------------


########################################
########Extra notes for keystone on VSM

#First, you need to upgrate six, using pip install -U six
#Copy keystonemiddleware to /usr/lib/python2.6/sites-packages
#Replace [filter:authtoken] in /etc/vsm/api-paste.ini

#[filter:authtoken]
#paste.filter_factory = keystonemiddleware.auth_token:filter_factory
#service_protocol = http
#service_host = 127.0.0.1
#service_port = 5000
#auth_host = 10.239.131.161#New keystone host IP
#auth_port = 35357
#auth_protocol = http
#admin_tenant_name = service
#admin_user = vsm
#admin_password = vsmservice #vsm service password
#signing_dir = /var/lib/vsm

#######################################
unset OS_USERNAME
unset OS_AUTH_KEY
unset OS_AUTH_TENANT
unset OS_STRATEGY
unset OS_AUTH_STRATEGY
unset OS_AUTH_URL
unset SERVICE_TOKEN
unset SERVICE_ENDPOINT
unset http_proxy
unset https_proxy
unset ftp_proxy


function get_field_value()
{
    if [ ! -f $1 ] || [ $# -ne 3 ];then
        return 1
    fi
    blockname=$2
    fieldname=$3

    begin_block=0
    end_block=0

    cat $1 | while read line
        do
            if [ "X$line" = "X[$blockname]" ];then
                begin_block=1
                continue
            fi

            if [ $begin_block -eq 1 ];then
                end_block=$(echo $line | awk 'BEGIN{ret=0} /^\[.*\]$/{ret=1} END{print ret}')
                if [ $end_block -eq 1 ];then
                        break
                fi

                need_ignore=$(echo $line | awk 'BEGIN{ret=0} /^#/{ret=1} /^$/{ret=1} END{print ret}')
                if [ $need_ignore -eq 1 ];then
                        continue
                fi
                field=$(echo $line | awk -F= '{gsub(" |\t","",$1); print $1}')
                value=$(echo $line | awk -F= '{gsub(" |\t","",$2); print $2}')

                if [ "X$fieldname" = "X$field" ];then
                        echo $value
                        break
                fi

            fi
        done

        return 0
}

OS_USERNAME=$(get_field_value deploy.ini Default OS_USERNAME)
OS_PASSWORD=$(get_field_value deploy.ini Default OS_PASSWORD)
OS_TENANT_NAME=$(get_field_value deploy.ini Default OS_TENANT_NAME)
KEYSTONE_HOST=$(get_field_value deploy.ini Default KEYSTONE_HOST)
AGENT_PASSWORD=$(get_field_value deploy.ini Default AGENT_PASSWORD)
VSM_HOST=$(get_field_value deploy.ini Default VSM_HOST)
KEYSTONE_VSM_SERVICE_PASSWORD=$(get_field_value deploy.ini Default KEYSTONE_VSM_SERVICE_PASSWORD)

echo $OS_USERNAME, $OS_PASSWORD, $OS_TENANT_NAME, $KEYSTONE_HOST, $AGENT_PASSWORD, $VSM_HOST, $KEYSTONE_VSM_SERVICE_PASSWORD


export OS_USERNAME=$OS_USERNAME
export OS_PASSWORD=$OS_PASSWORD
export OS_TENANT_NAME=$OS_TENANT_NAME
export OS_AUTH_URL=http://127.0.0.1:5000/v2.0
export OS_ENDPOINT=http://127.0.0.1:35357/v2.0
export KEYSTONE_HOST=$KEYSTONE_HOST

KEYSTONE_AUTH_HOST=$KEYSTONE_HOST
KEYSTONE_AUTH_PORT=35357
KEYSTONE_AUTH_PROTOCOL=http
KEYSTONE_SERVICE_HOST=$KEYSTONE_HOST
KEYSTONE_SERVICE_PORT=5000
KEYSTONE_SERVICE_PROTOCOL=http
SERVICE_ENDPOINT=http://$KEYSTONE_HOST:35357/v2.0

#VSM controller node IP
VSM_HOST=$VSM_HOST
keyrc=~/keyrc

#---------------------------------------------------
# Create VSM User in Keystone
#---------------------------------------------------

get_id() {
    echo `"$@" | awk '/ id / { print $4 }'`
}

get_tenant() {
    var=$1;
    pw=${!var}
    pw=`keystone tenant-list | grep $2 | awk '{print $2}'`
    eval "$var=$pw"
}

get_role() {
    var=$1;
    pw=${!var}
    pw=`keystone role-list | grep $2 | awk '{print $2}'`
    eval "$var=$pw"
}

# Grab a numbered field from python prettytable output
# Fields are numbered starting with 1
# Reverse syntax is supported: -1 is the last field, -2 is second to last, etc.
# get_field field-number
function get_field {
    local data field
    while read data; do
        if [ "$1" -lt 0 ]; then
            field="(\$(NF$1))"
        else
            field="\$$(($1 + 1))"
        fi
        echo "$data" | awk -F'[ \t]*\\|[ \t]*' "{print $field}"
    done
}

# Gets or creates project
# Usage: get_or_create_project <name>
function get_or_create_project {
    # Gets project id
    local project_id=$(
        # Gets project id
        openstack project show $1 -f value -c id 2>/dev/null ||
        # Creates new project if not exists
        openstack project create $1 -f value -c id
    )
    echo $project_id
}

# Gets or creates user
# Usage: get_or_create_user <username> <password> <project> [<email>]
function get_or_create_user {
    if [[ ! -z "$4" ]]; then
        local email="--email=$4"
    else
        local email=""
    fi
    # Gets user id
    local user_id=$(
        # Gets user id
        openstack user show $1 -f value -c id 2>/dev/null ||
        # Creates new user
        openstack user create \
            $1 \
            --password "$2" \
            --project $3 \
            $email \
            -f value -c id
    )
    echo $user_id
}

# Gets or creates role
# Usage: get_or_create_role <name>
function get_or_create_role {
    local role_id=$(
        # Gets role id
        openstack role show $1 -f value -c id 2>/dev/null ||
        # Creates role if not exists
        openstack role create $1 -f value -c id
    )
    echo $role_id
}

# Gets or adds user role
# Usage: get_or_add_user_role <role> <user> <project>
function get_or_add_user_role {
    # Gets user role id
    local user_role_id=$(openstack user role list \
        $2 \
        --project $3 \
        --column "ID" \
        --column "Name" \
        | grep " $1 " | get_field 1)
    if [[ -z "$user_role_id" ]]; then
        # Adds role to user
        user_role_id=$(openstack role add \
            $1 \
            --user $2 \
            --project $3 \
            | grep " id " | get_field 2)
    fi
    echo $user_role_id
}

# Gets or creates service
# Usage: get_or_create_service <name> <type> <description>
function get_or_create_service {
    # Gets service id
    local service_id=$(
        # Gets service id
        openstack service show $1 -f value -c id 2>/dev/null ||
        # Creates new service if not exists
        openstack service create \
            $1 \
            --name=$2 \
            --description="$3" \
            -f value -c id
    )
    echo $service_id
}

# Gets or creates endpoint
# Usage: get_or_create_endpoint <service> <region> <publicurl> <adminurl> <internalurl>
function get_or_create_endpoint {
    # Gets endpoint id
    local endpoint_id=$(openstack endpoint list \
        --column "ID" \
        --column "Region" \
        --column "Service Name" \
        | grep " $2 " \
        | grep " $1 " | get_field 1)
    if [[ -z "$endpoint_id" ]]; then
        # Creates new endpoint
        endpoint_id=$(openstack endpoint create \
            $1 \
            --region $2 \
            --publicurl $3 \
            --adminurl $4 \
            --internalurl $5 \
            | grep " id " | get_field 2)
    fi
    echo $endpoint_id
}
##############################################################
#########Create agent realated information in keystone########
###############################################################



AGENT_TENANT=$(get_or_create_project agent)
echo "export AGENT_TENANT=$AGENT_TENANT" >> $keyrc

AGENT_USER=$(get_or_create_user agent "$AGENT_PASSWORD" "$AGENT_TENANT" agent@intel.com)

AGENT_ROLE=$(get_or_create_role agent)
echo "export AGENT_ROLE=$AGENT_ROLE" >> $keyrc

get_or_add_user_role $AGENT_ROLE $AGENT_USER $AGENT_TENANT

###################################

get_tenant SERVICE_TENANT service
get_role ADMIN_ROLE admin


if [[ `keystone user-list | grep vsm | wc -l` -eq 0 ]]; then
    VSM_USER=$(get_or_create_user vsm "$KEYSTONE_VSM_SERVICE_PASSWORD" $SERVICE_TENANT vsm@example.com)
    get_or_add_user_role $ADMIN_ROLE $VSM_USER $SERVICE_TENANT

    VSM_SERVICE=$(get_or_create_service vsm vsm "VSM Service")
    get_or_create_endpoint $VSM_SERVICE RegionOne \
        "http://$VSM_HOST:8778/v1/\$(tenant_id)s" \
        "http://$VSM_HOST:8778/v1/\$(tenant_id)s" \
        "http://$VSM_HOST:8778/v1/\$(tenant_id)s"
fi

unset SERVICE_TOKEN
unset SERVICE_ENDPOINT



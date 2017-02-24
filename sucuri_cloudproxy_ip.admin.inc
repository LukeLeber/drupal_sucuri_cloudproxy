<?php

/**
 * @file
 * Provides remote whitelist/blacklist maintenance capabilities
 */

/** @var string A regular expression that matches IPV4 addresses and CIDR ranges */
const REGEX_MATCH_IPV4_WITH_CIDR = '/^(?=\d+\.\d+\.\d+\.\d+($|\/))(([1-9]?\d|1\d\d|2[0-4]\d|25[0-5])\.?){4}(\/([0-9]|[1-2][0-9]|3[0-2]))?$/';

/** @var string A regular expression that matches IPV6 addresses and CIDR ranges */
const REGEX_MATCH_IPV6_WITH_CIDR = '/^s*((([0-9A-Fa-f]{1,4}:){7}([0-9A-Fa-f]{1,4}|:))|(([0-9A-Fa-f]{1,4}:){6}(:[0-9A-Fa-f]{1,4}|((25[0-5]|2[0-4]d|1dd|[1-9]?d)(.(25[0-5]|2[0-4]d|1dd|[1-9]?d)){3})|:))|(([0-9A-Fa-f]{1,4}:){5}(((:[0-9A-Fa-f]{1,4}){1,2})|:((25[0-5]|2[0-4]d|1dd|[1-9]?d)(.(25[0-5]|2[0-4]d|1dd|[1-9]?d)){3})|:))|(([0-9A-Fa-f]{1,4}:){4}(((:[0-9A-Fa-f]{1,4}){1,3})|((:[0-9A-Fa-f]{1,4})?:((25[0-5]|2[0-4]d|1dd|[1-9]?d)(.(25[0-5]|2[0-4]d|1dd|[1-9]?d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){3}(((:[0-9A-Fa-f]{1,4}){1,4})|((:[0-9A-Fa-f]{1,4}){0,2}:((25[0-5]|2[0-4]d|1dd|[1-9]?d)(.(25[0-5]|2[0-4]d|1dd|[1-9]?d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){2}(((:[0-9A-Fa-f]{1,4}){1,5})|((:[0-9A-Fa-f]{1,4}){0,3}:((25[0-5]|2[0-4]d|1dd|[1-9]?d)(.(25[0-5]|2[0-4]d|1dd|[1-9]?d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){1}(((:[0-9A-Fa-f]{1,4}){1,6})|((:[0-9A-Fa-f]{1,4}){0,4}:((25[0-5]|2[0-4]d|1dd|[1-9]?d)(.(25[0-5]|2[0-4]d|1dd|[1-9]?d)){3}))|:))|(:(((:[0-9A-Fa-f]{1,4}){1,7})|((:[0-9A-Fa-f]{1,4}){0,5}:((25[0-5]|2[0-4]d|1dd|[1-9]?d)(.(25[0-5]|2[0-4]d|1dd|[1-9]?d)){3}))|:)))(%.+)?s*(\/([0-9]|[1-9][0-9]|1[0-1][0-9]|12[0-8]))?$/';

/** @var string The name of the CSRF token field */
const FORM_TOKEN_FIELD_NAME = 'form_token';

/**
 * Does this request contain a valid CSRF token?
 * 
 * @param string $value the seed off which the token is partially based
 * 
 * @param $query the array to be processed (defaults to $_GET)
 * 
 * @return boolean If true, then this request was (probably) not forged.
 */
function _validate_csrf_token($value, $query = NULL) {
  $query_parameters = drupal_get_query_parameters($query);
  if(!isset($query_parameters[FORM_TOKEN_FIELD_NAME])) {
    return false;
  }
  return drupal_valid_token($query_parameters[FORM_TOKEN_FIELD_NAME], $value);
}

/**
 * Generates a CSRF protected delete link for the provided entry
 * 
 * @param string $type the type of entry (whitelist | blacklist)
 * 
 * @param string $ip the IP address
 * 
 * @return string
 */
function _generate_secure_delete_link($type, $ip) {
    $text = t('Delete');
    $path = "admin/config/people/sucuri_cloudproxy_ip_filter/{$type}/delete/nojs/{$ip}";
    $token = drupal_get_token("sucuri_cloudproxy_ip_filter_{$type}" . $ip);
    $options = array(
      'attributes' => array('class' => "use-ajax {$type}-delete-link"), 
      'query' => array(FORM_TOKEN_FIELD_NAME => $token) + drupal_get_destination()
    );
    
    return l($text, $path, $options);
}

/**
 * Retrieves the complete list of IP addresses in the specified list
 * 
 * @param string $type the type of list (whitelist | blacklist)
 * 
 * @return array
 */
function _fetch_ips($type) {

  /// required for ajax enabled delete links
  drupal_add_library('system', 'drupal.ajax');
  
  /// required for sucuri api calls
  module_load_include('inc', 'sucuri_cloudproxy', 'sucuri_cloudproxy_api.admin');

  $rows = array();
  $result = sucuri_cloudproxy_api_show_settings("{$type}_list");
  foreach($result as $ip) {
    $rows[] = array(
      'data' => array($ip, _generate_secure_delete_link($type, $ip)),
      'class' => array("sucuri-cloudproxy-{$type}-ip", "sucuri-cloudproxy-{$type}-ip-" . ip2long($ip))
    );
  }
  return $rows;
}

/**
 * Renders an table containing the provided IPs
 * 
 * @param string $caption the table caption
 * 
 * @param array $rows the rows to render
 * 
 * @param string $empty_text the text to display if there are no rows
 * 
 * @return string
 */
function _sucuri_cloudproxy_ip_filter_list($caption, $rows, $empty_text) {
    
  /// ensure that the api keys are properly configured
  if(!variable_get('sucuri_cloudproxy_site_key') || !variable_get('sucuri_cloudproxy_secret_key')) {
    drupal_set_message(t('A valid set of API keys must be configured prior to managing IP filtering.'), 'warning');
    drupal_goto('admin/config/system/sucuri_cloudproxy');
    return;
  }
    
  return theme_table(
    array(
      'header'     => array('IP Address', 'Operations'),
      'rows'       => $rows, 
      'caption'    => $caption,
      'attributes' => array(
        'id' => 'ip_address_table',
      ),
      'sticky'     => FALSE,
      'colgroups'  => array(),
      'empty'      => $empty_text,
    )
  );
}

/**
 * Implements hook_form_validate();
 */
function sucuri_cloudproxy_ip_filter_add_form_validate($form, &$form_state) {
  if(!_validate_csrf_token('sucuri_cloudproxy_ip_filter_add_form', $_POST)) {
    form_set_error(FORM_TOKEN_FIELD_NAME, t('Your security token has expired.  Please refresh this page.'));
  } else if(!$form_state['values']['my_ip']) {
    $ip = $form_state['values']['ip'];
    if(!preg_match(REGEX_MATCH_IPV4_WITH_CIDR, $ip) && 
       !preg_match(REGEX_MATCH_IPV6_WITH_CIDR, $ip)) {
      form_set_error('ip', t('An invalid IP address or CIDR range was provided.'));
    }
  }
}

/**
 * Implements hook_form_callback();
 */
function sucuri_cloudproxy_ip_filter_add_form_callback($form, &$form_state) {
  $commands = array();
  if(form_get_errors()) {
    $form_state['rebuild'] = TRUE;
  }
  
  $type = $form_state['values']['type'];
  
  $commands[] = ajax_command_remove('div.messages');
  $commands[] = ajax_command_before('.region-content', theme('status_messages'));
  $commands[] = ajax_command_replace(NULL, render($form));
  
  $replacement = NULL;
  switch($type) {
    case 'whitelist':
      $replacement = _sucuri_cloudproxy_ip_filter_list(
        t('Whitelisted IP Addresses'),
        _fetch_ips('whitelist'),
        t('There are no IP addresses whitelisted at this time.')
      );
      break;
    case 'blacklist':
      $replacement = _sucuri_cloudproxy_ip_filter_list(
        t('Blacklisted IP Addresses'),
        _fetch_ips('blacklist'),
        t('There are no IP addresses blacklisted at this time.')
      );
      break;
  }
  if($replacement) {
    $commands[] = ajax_command_replace('#ip_address_table', $replacement);
  }
  
  return array(
    '#type' => 'ajax', 
    '#commands' => $commands
  );
}

/**
 * Implements hook_form_submit();
 */
function sucuri_cloudproxy_ip_filter_add_form_submit($form, &$form_state) {

  /// required for sucuri api calls
  module_load_include('inc', 'sucuri_cloudproxy', 'sucuri_cloudproxy_api.admin');
  
  $my_ip = (bool)$form_state['values']['my_ip'];
  $type = $form_state['values']['type'];
  
  $result = FALSE;
  
  switch($type) {
    case 'whitelist':
      $result = (bool)sucuri_cloudproxy_api_whitelist_ip($my_ip ? NULL : $form_state['values']['ip']);     
      break;
    case 'blacklist':
      $result = (bool)sucuri_cloudproxy_api_blacklist_ip($form_state['values']['ip']);
      break;
    default:
      drupal_set_message(t('Unknown list type @list', array('@list' => $form_state['values']['type'])), 'error');
  }
  $form_state['values']['success'] = $result;
}

/**
 * Implements hook_form();
 */
function sucuri_cloudproxy_ip_filter_add_form($form, &$form_state, $type) {

  $ip = array(
    '#type' => 'textfield',
    '#title' => t('Custom IP Address or CIDR Range'),
    '#attributes' => array(
      'placeholder' => t('127.0.0.1 | 192.168.194.0/24'),
    ),
  );
  
  $submit = array(
    '#type' => 'submit',
    '#title' => t('Add'),
    '#value' => t('Add'),
    '#ajax' => array(
      'callback' => 'sucuri_cloudproxy_ip_filter_add_form_callback',
      'wrapper' => 'add_form_wrapper',
    )
  );
  
  $main_fieldset = array(
    '#type' => 'fieldset',
    '#description' => t('New @type entry', array('@type' => $type)),
    'type' => array(
      '#type' => 'hidden',
      '#value' => $type,
    )
  );

  if($type == 'whitelist') {
    $main_fieldset['my_ip'] = array(
      '#type' => 'checkbox',
      '#title' => t('Whitelist My IP'),
    );
    $main_fieldset['ip_container'] = array(
      '#type' => 'container',
      '#states' => array(
        'visible' => array(
          'input[name="my_ip"]' => array('checked' => FALSE),
        ),
      ),
      'ip' => $ip,
    );
  } else {
      $main_fieldset['ip'] = $ip;
  }
  
  $main_fieldset['submit'] = $submit;
  
  
  
  $form[] = array(
    '#id'     => 'sucuri_cloudproxy_ip_filter_add_form',
    '#prefix' => '<div id="add_form_wrapper">',
    '#suffix' => '</div>',
    'main_fieldset' => $main_fieldset,
  );
  
  return $form;
}

/**
 * Renders the IP white-list page
 * 
 * @return string
 */
function sucuri_cloudproxy_ip_filter_whitelist() {
  $whitelist = _sucuri_cloudproxy_ip_filter_list(
    t('Whitelisted IP Addresses'),
    _fetch_ips('whitelist'),
    t('There are no IP addresses whitelisted at this time.')
  );
  
  /// required for ajax enabled delete links
  drupal_add_library('system', 'jquery.form');
  
  $form = drupal_get_form('sucuri_cloudproxy_ip_filter_add_form', 'whitelist');
  return drupal_render($form) . $whitelist;
}

/**
 * Renders the IP black-list page
 * 
 * @return string
 */
function sucuri_cloudproxy_ip_filter_blacklist() {
  $blacklist = _sucuri_cloudproxy_ip_filter_list(
    t('Blacklisted IP Addresses'),
    _fetch_ips('blacklist'),
    t('There are no IP addresses blacklisted at this time.')
  );
  
  /// required for ajax enabled delete links
  drupal_add_library('system', 'jquery.form');
  
  $form = drupal_get_form('sucuri_cloudproxy_ip_filter_add_form', 'blacklist');
  
  return drupal_render($form) . $blacklist;
}

/**
 * Deletes a whitelist entry
 * @param string $is_ajax set to the constant 'ajax' if this was an ajax request
 * 
 * @param string $ip the ip address or CIDR range to delete
 * 
 * @return mixed returns an array of commands if the request was sent via ajax, else void.
 */
function sucuri_cloudproxy_ip_filter_whitelist_delete($is_ajax, $ip) {
    
  if(!_validate_csrf_token('sucuri_cloudproxy_ip_filter_whitelist' . $ip)) {
    return MENU_ACCESS_DENIED;
  }
  module_load_include('inc', 'sucuri_cloudproxy', 'sucuri_cloudproxy_api.admin');
  
  $result = sucuri_cloudproxy_api_delete_whitelisted_ip($ip);
  
  if($is_ajax === 'ajax') {
    $commands = array(
      ajax_command_remove('div.messages'),
      ajax_command_before('.region-content', theme('status_messages')),
    );
    if($result) {
      $commands[] = ajax_command_replace(
        '#ip_address_table', 
        _sucuri_cloudproxy_ip_filter_list(
          t('Whitelisted IP Addresses'),
          _fetch_ips('whitelist'),
          t('There are no IP addresses whitelisted at this time.')
        )
      );
    }
    return array(
      '#type' => 'ajax', 
      '#commands' => $commands
    );
  } else {
    drupal_goto();
  }
}

/**
 * Deletes a whitelist entry
 * @param string $is_ajax set to the constant 'ajax' if this was an ajax request
 * 
 * @param string $ip the ip address or CIDR range to delete
 * 
 * @return mixed returns an array of commands if the request was sent via ajax, else void.
 */
function sucuri_cloudproxy_ip_filter_blacklist_delete($is_ajax, $ip) {
  if(!_validate_csrf_token('sucuri_cloudproxy_ip_filter_blacklist' . $ip)) {
    return MENU_ACCESS_DENIED;
  }

  module_load_include('inc', 'sucuri_cloudproxy', 'sucuri_cloudproxy_api.admin');
  
  $result = sucuri_cloudproxy_api_delete_blacklisted_ip($ip);
  
  if($is_ajax === 'ajax') {
    $commands = array(
      ajax_command_remove('div.messages'),
      ajax_command_before('.region-content', theme('status_messages')),
    );
    if($result) {
      $commands[] = ajax_command_replace(
        '#ip_address_table', 
        _sucuri_cloudproxy_ip_filter_list(
          t('Blacklisted IP Addresses'),
          _fetch_ips('blacklist'),
          t('There are no IP addresses blacklisted at this time.')
        )
      );
    }
    return array(
      '#type' => 'ajax', 
      '#commands' => $commands
    );
  } else {
    drupal_goto();
  }
}

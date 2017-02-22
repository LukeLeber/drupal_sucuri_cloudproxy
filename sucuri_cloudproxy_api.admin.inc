<?php

/**
 * Performs an API call with the provided parameters
 * 
 * @param array $params
 * 
 * @return array
 * 
 * @throws Exception if any error occurs with the API call
 */
function _sucuri_cloudproxy_call_api($params) {
  if (!function_exists('curl_version')) {
    throw new Exception(t('Sorry, but cURL must be installed and enabled in order to utilize this functionality.'), 'error');
  }
  
  $handle = curl_init();
  if ($handle === FALSE) {
    $internal_message = 'cURL failed to initialize on line ' . __LINE__ . ' in ' . __FILE__ . ': ' . curl_error($handle);
    watchdog('sucuri_cloudproxy', $internal_message, WATCHDOG_ERROR);
    throw new Exception(t('The system is unable to process your request at this time.'));
  }
  
  $query = http_build_query(
    array(
      'k' => variable_get('sucuri_cloudproxy_site_key'),
      's' => variable_get('sucuri_cloudproxy_secret_key')
    ) + $params);

  if (curl_setopt_array($handle, array(
    CURLOPT_URL            => 'https://waf.sucuri.net/api?v2',
    CURLOPT_REFERER => url(null, array('absolute' => true)),
    CURLOPT_USERAGENT => 'Sucuri CloudProxy Drupal Plugin/1.0 (+https://www.drupal.org/sandbox/lleber/2831968)',
    CURLOPT_POST           => TRUE,
    CURLOPT_POSTFIELDS     => $query,
    CURLOPT_HEADER         => FALSE,
    CURLOPT_SSL_VERIFYHOST => 2,
    CURLOPT_SSL_VERIFYPEER => 1,
    CURLOPT_PORT           => 443,
    CURLOPT_RETURNTRANSFER => TRUE,
  )) === FALSE) {
    $internal_message = 'cURL failed to set options on line ' . __LINE__ . ' in ' . __FILE__ . ': ' . curl_error($handle);
    watchdog('sucuri_cloudproxy', $internal_message, WATCHDOG_ERROR);
    throw new Exception(t('The system is unable to process your request at this time.'));
  }  

  $response = curl_exec($handle);
  
  if ($response === FALSE) {
    $internal_message = 'cURL failed to execute on line ' . __LINE__ . ' in ' . __FILE__ . ': ' . curl_error($handle);
    watchdog('sucuri_cloudproxy', $internal_message, WATCHDOG_ERROR);
    throw new Exception(t('The system is unable to process your request at this time.'));
  }

  $result = json_decode($response, TRUE);
  
  if (!$result['status']) {
    $messages = implode('/', $result['messages']);
    throw new Exception(t('One or more unexpected errors have occurred: @messages', array('@messages' => $messages)));
  }
  if(isset($result['messages'])) {
      foreach($result['messages'] as $message) {
          drupal_set_message($message);
      }
  }
  return $result;
}

/**
 * Performs a 'show_settings' API call, optionally filtering out the desired property
 * 
 * @param string $property the optional property to filter
 * 
 * @return mixed
 *  If the $property parameter is utilized, then the value of $property is 
 *  returned, otherwise the entire response is returned.  If any error occurs, 
 *  then a flash message is set and FALSE is returned.
 * 
 */
function sucuri_cloudproxy_api_show_settings($property = null) {
  try {
    $params = array('a' => 'show_settings');
    
    $result = _sucuri_cloudproxy_call_api($params);
    
    if($property) {
      if(isset($result['output'][$property])) {
        return $result['output'][$property];          
      } else {
        throw new Exception(t("Property $property was not found."));          
      }
    }
    return $result;
  } catch (Exception $ex) {
    drupal_set_message($ex->getMessage(), 'error');
  }
  return false;
}

/**
 * Performs a 'clear_cache' API call, optionally clearing a single file.  If 
 * no file is provided, then all files will be cleared.
 * 
 * @param string $file the optional file to clear
 * 
 * @return mixed
 *   This method normally returns the entire API response array.  However if 
 *   any error occurs, then a flash message is set and FALSE is returned.
 */
function sucuri_cloudproxy_api_clear_cache($file = null) {
  try {
    $params = array('a' => 'clear_cache');
    if($file) {
      $params['file'] = $file;
    }
    return _sucuri_cloudproxy_call_api($params);
  } catch (Exception $ex) {
    drupal_set_message($ex->getMessage(), 'error');
  }
  return false;
}

/**
 * Performs a 'whitelist_ip' API call.  If no ip or CIDR range is provided, 
 * then the IP of the client will be whitelisted.
 * 
 * @param string $ip the IP address or CIDR range to whitelist
 * 
 * @return mixed
 *   This method normally returns the entire API response array.  However if 
 *   any error occurs, then a flash message is set and FALSE is returned.
 */
function sucuri_cloudproxy_api_whitelist_ip($ip = null) {
  try {
    $params = array('a' => 'whitelist_ip');
    if($ip) {
        $params['ip'] = $ip;
    }
    return _sucuri_cloudproxy_call_api($params);
  } catch (Exception $ex) {
    drupal_set_message($ex->getMessage(), 'error');
  }
  return false;    
}

/**
 * Performs a 'delete_whitelist_ip' API call.
 * 
 * @param string $ip the IP address or CIDR range to remove from the whitelist
 * 
 * @return mixed
 *   This method normally returns the entire API response array.  However if 
 *   any error occurs, then a flash message is set and FALSE is returned.
 */
function sucuri_cloudproxy_api_delete_whitelisted_ip($ip) {
  try {
    $params = array('a' => 'delete_whitelist_ip', 'ip' => $ip);
    return _sucuri_cloudproxy_call_api($params);
  } catch (Exception $ex) {
    drupal_set_message($ex->getMessage(), 'error');
  }
  return false;
}

/**
 * Performs a 'blacklist_ip' API call.
 * 
 * @param string $ip the IP address or CIDR range to add to the blacklist
 * 
 * @return mixed
 *   This method normally returns the entire API response array.  However if 
 *   any error occurs, then a flash message is set and FALSE is returned.
 */
function sucuri_cloudproxy_api_blacklist_ip($ip) {
  try {
    $params = array('a' => 'blacklist_ip', 'ip' => $ip);
    return _sucuri_cloudproxy_call_api($params);
  } catch (Exception $ex) {
    drupal_set_message($ex->getMessage(), 'error');
  }
  return false;    
}

/**
 * Performs a 'delete_blacklist_ip' API call.
 * 
 * @param string $ip the IP address or CIDR range to remove from the blacklist
 * 
 * @return mixed
 *   This method normally returns the entire API response array.  However if 
 *   any error occurs, then a flash message is set and FALSE is returned.
 */
function sucuri_cloudproxy_api_delete_blacklisted_ip($ip) {
  try {
    $params = array('a' => 'delete_blacklist_ip', 'ip' => $ip);
    return _sucuri_cloudproxy_call_api($params);
  } catch (Exception $ex) {
    drupal_set_message($ex->getMessage(), 'error');
  }
  return false;
}

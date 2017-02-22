<?php

/** @var string The name of the optional form encryption module */
const FORM_ENCRYPTION_MODULE = 'encryptfapi';

/**
 * Implements hook_form();
 */
function sucuri_cloudproxy_config_form($form, &$form_state) {

  $site_key = array(
    '#type'          => 'textfield',
    '#title'         => t('Site Key'),
    '#default_value' => variable_get('sucuri_cloudproxy_site_key'),
    '#size'          => 32,
    '#maxlength'     => 32,
    '#description'   => t('The site key issued by Sucuri for this domain.'),
    '#required'      => TRUE,
  );

  $secret_key = array(
    '#type'          => 'textfield',
    '#title'         => t('Secret Key'),
    '#default_value' => variable_get('sucuri_cloudproxy_secret_key'),
    '#size'          => 32,
    '#maxlength'     => 32,
    '#description'   => t('The secret key issued by Sucuri for this domain.'),
    '#required'      => TRUE,
  );
  
  $key_configuration = array(
    '#type'        => 'fieldset',
    '#title'       => t('Key Configuration'),
    '#collapsible' => TRUE,
    '#collapsed'   => FALSE,
  );

  if(!module_exists(FORM_ENCRYPTION_MODULE)) {
    $site_key['#encrypt'] = TRUE;
    $secret_key['#encrypt'] = TRUE;
  }
  
  $key_configuration['sucuri_cloudproxy_site_key'] = $site_key;
  $key_configuration['sucuri_cloudproxy_secret_key'] = $secret_key;
  
  $form['key_configuration'] = $key_configuration;
  return system_settings_form($form);
}

/**
 * Implements hook_form_validate();
 */
function sucuri_cloudproxy_config_form_validate($form, &$form_state) {
  foreach (array(
    'sucuri_cloudproxy_site_key'   => 'site key',
    'sucuri_cloudproxy_secret_key' => 'secret key',
  ) as $k => $v) {

    if (!preg_match('/^[a-fA-F0-9]{32}$/', $form_state['values'][$k])) {
      form_set_error($k, t('Your @field should be a 32 character long hexadecimal string.', array('@field' => $v)));
    }
  }
}

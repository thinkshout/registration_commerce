<?php

/**
 * @file
 * Form definitions and callbacks for Registration Commerce.
 */

/**
 * Form callback: edit Registration Commerce settings on a registration type.
 *
 * @param $registration_type
 *   The registration_type object to apply the settings to.
 */
function registration_commerce_registration_type_form($form, &$form_state, $registration_type) {
  $form_state['registration_type_bundle'] = $registration_type->name;
  // Grab the existing settings and establish defaults:
  $comreg_settings = variable_get('commerce_registration_type_settings', array());
  if (isset($comreg_settings[$registration_type->name])) {
    $defaults = $comreg_settings[$registration_type->name];
  }
  else {
    $defaults = array();
  }

  $form['price_defaults'] = array(
    '#type' => 'fieldset',
    '#title' => t('Default Price Settings'),
    '#collapsible' => FALSE,
    '#tree' => TRUE,
  );
  $form['price_defaults']['amount'] = array(
    '#type' => 'textfield',
    '#title' => t('Amount'),
    '#default_value' => isset($defaults['price_defaults']['amount']) ? $defaults['price_defaults']['amount'] / 100 : 0,
    '#element_validate' => array('element_validate_number'),
  );
  $form['price_defaults']['currency_code'] = array(
    '#type' => 'select',
    '#title' => t('Currency'),
    '#options' => commerce_currency_code_options_list(),
    '#default_value' => isset($defaults['price_defaults']['currency_code']) ? $defaults['price_defaults']['currency_code'] : commerce_default_currency(),
  );
  // Load the current registration states to update the form options:
  $states = registration_get_states();
  $options = array();
  foreach ($states as $name => $state) {
    $options[$name] = $state->label;
  }
  $form['purchasable_states'] = array(
    '#title' => t('Which states should allow for a purchase on @label registrations?',
      array('@label' => $registration_type->label)),
    '#type' => 'checkboxes',
    '#options' => $options,
    '#default_value' => $defaults['purchasable_states'],
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Validation callback for registration_form().
 */
function registration_commerce_registration_type_form_validate($form, &$form_state) {
  // Unused.
}

/**
 * Submit callback for registration_form().
 */
function registration_commerce_registration_type_form_submit($form, &$form_state) {
  // @todo should this be stored in the variable table or somewhere else?
  $settings = variable_get('commerce_registration_type_settings', array());
  $settings[$form_state['registration_type_bundle']] = array(
    'price_defaults' => $form_state['values']['price_defaults'],
    'purchasable_states' => $form_state['values']['purchasable_states'],
  );
  $settings[$form_state['registration_type_bundle']]['price_defaults']['amount'] *= 100;
  variable_set('commerce_registration_type_settings', $settings);
  drupal_set_message(t('Registration Commerce settings have been saved.'));
}

<?php
/**
 * @file
 * Registration Commerce rules file.
 */

/**
 * Implements hook_rules_condition_info().
 */
function registration_commerce_rules_condition_info() {
  $conditions = array();

  $conditions['registration_commerce_item_can_register'] = array(
    'label' => t('Registration referenced by a line item is available for registration.'),
    'group' => t('Registration'),
    'parameter' => array(
      'commerce_line_item' => array(
        'type' => 'commerce_line_item',
        'label' => t('Commerce Line Item'),
        'description' => t('The Commerce Line Item referencing the registration to check.'),
      ),
    ),
    'callbacks' => array(
      'execute' => 'registration_commerce_item_can_register',
    ),
  );

  $conditions['registration_commerce_registration_has_extant_line_item'] = array(
    'label' => t('Registration is contained in any other cart.'),
    'group' => t('Registration'),
    'parameter' => array(
      'commerce_line_item' => array(
        'type' => 'commerce_line_item',
        'label' => t('Commerce Line Item'),
        'description' => t('The Line Item to compare for duplicates.'),
      ),
    ),
    'callbacks' => array(
      'execute' => 'registration_commerce_registration_has_extant_line_item',
    ),
  );

  return $conditions;
}

/**
 * Implements hook_rules_action_info().
 */
function registration_commerce_rules_action_info() {
  $actions = array();

  $actions['registration_commerce_set_state'] = array(
    'label' => t('Set state for all registrations in an order'),
    'group' => t('Registration'),
    'parameter' => array(
      'commerce_order' => array(
        'type' => 'commerce_order',
        'label' => t('Commerce Order'),
        'description' => t('All registrations on this order will have their state changed.'),
      ),
      'registration_state' => array(
        'type' => 'text',
        'label' => t('Registration State'),
        'description' => t('Registrations will be set to this state.'),
        'options list' => 'registration_get_states_options',
      ),
    ),
  );

  return $actions;
}

/**
 * @defgroup registration_commerce_rules_conditions Rules Conditions
 * @{
 * Condition callbacks for Registration Commerce Registration rules.
 */

/**
 * Condition callback.
 *
 * Checks if any carts contain a line item matching this registration.
 *
 * @param Registration $registration
 *
 * @return int
 *   Returns the ID of another order containing this item, or 0
 */
function registration_commerce_registration_has_extant_line_item($line_item) {
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'commerce_line_item')
    ->entityCondition('bundle', 'registration')
    // Avoid a false positive on this line item:
    ->entityCondition('entity_id', $line_item->line_item_id, '!=')
    // todo: use wrapper:
    ->fieldCondition(REGISTRATION_COMMERCE_FIELD, 'target_id', $line_item->{REGISTRATION_COMMERCE_FIELD}['und'][0]['target_id'], '=')
    ->addMetaData('account', user_load(1));

  $result = $query->execute();
  if (empty($result)) {
    return 0;
  }
  $first_result = array_shift($result['commerce_line_item']);
  $extant_line_item = commerce_line_item_load($first_result->line_item_id);
  // Now we have all the line items that point to this registration.
  return $extant_line_item->order_id;
}

/**
 * Condition callback.
 *
 * Checks if registration associated with the line item product has registration capacity left.
 *
 * @param CommerceLineItem $line_item
 *   Commerce Line Item to check.
 *
 * @return bool
 *   Boolean TRUE if the associated registration can be made active, if it's not already.
 */
function registration_commerce_item_can_register($line_item) {
  $line_item_wrapper = entity_metadata_wrapper('commerce_line_item', $line_item);
  if ($line_item->type != 'registration') {
    return TRUE;
  }

  $status = registration_status(
    $line_item_wrapper->{REGISTRATION_COMMERCE_FIELD}->entity_type->value(),
    $line_item_wrapper->{REGISTRATION_COMMERCE_FIELD}->entity_id->value(),
    FALSE,
    1,
    $line_item_wrapper->{REGISTRATION_COMMERCE_FIELD}->registration_id->value()
  );

  return $status;
}

/**
 * @} End of "defgroup registration_commerce_rules_conditions".
 */

/**
 * @defgroup registration_commerce_rules_actions Rules Actions
 * @{
 * Action callbacks for Registration Commerce Registration rules.
 */

/**
 * Action callback.
 *
 * Sets all registrations on an order to a given state.
 */
function registration_commerce_set_state($order, $registration_state) {
  foreach (entity_metadata_wrapper('commerce_order', $order)->commerce_line_items as $delta => $line_item_wrapper) {
    if ($line_item_wrapper->type->value() == 'registration') {
      $registration = registration_load($line_item_wrapper->{REGISTRATION_COMMERCE_FIELD}->registration_id->value());
      $registration->state = $registration_state;
      registration_save($registration);
    }
  }
}

/**
 * @} End of "defgroup registration_commerce_rules_actions".
 */

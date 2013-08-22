<?php
  /**
   * @file
   * registration_commerce.rules_defaults.inc
   */

  /**
   * Implements hook_default_rules_configuration().
   */
function registration_commerce_default_rules_configuration() {

  // Add a reaction rule to set the price of a registration product based on the registration itself
  $rule = rules_reaction_rule();

  $rule->label = t('Set registration product price from referenced registration');
  $rule->active = TRUE;
  $rule->weight = 5;
  $rule->tags = array("Commerce Cart", "Registration Commerce");
  $rule
    ->event('commerce_product_calculate_sell_price')
    ->condition('entity_has_field', array(
    'entity:select' => 'commerce-line-item',
    'field' => REGISTRATION_COMMERCE_FIELD,
  ))
    ->condition('data_is', array(
    'data:select' => 'commerce-line-item:registration-commerce-reg:amount',
    'op' => '>',
    'value' => '0',
  ))
    ->action('commerce_line_item_unit_price_amount', array(
      'commerce_line_item:select' => 'commerce-line-item',
      'amount:select' => 'commerce-line-item:registration-commerce-reg:amount',
      'component_name' => 'base_price',
      'round_mode' => '1',
  ))
    ->action('commerce_line_item_unit_price_currency_code', array(
      'commerce_line_item:select' => 'commerce-line-item',
      'currency_code:select' => 'commerce-line-item:registration-commerce-reg:currency',
  ));

  $rules['registration_commerce_product_price_from_registration'] = $rule;

  // Add a reaction rule to unset the price of registration line items where the
  // registration can't be made active.
  $rule = rules_reaction_rule();

  $rule->label = t('Unset the price of unavailable registrations in the cart');
  $rule->active = TRUE;
  $rule->weight = 10;
  $rule->tags = array("Commerce Cart", "Registration Commerce");
  $rule
    ->event('commerce_product_calculate_sell_price')
    ->condition(rules_condition('data_is_empty', array(
    'data:select' => 'commerce-line-item:line-item-id',
  ))->negate())
    ->condition('entity_has_field', array(
    'entity:select' => 'commerce-line-item',
    'field' => REGISTRATION_COMMERCE_FIELD,
  ))
    ->condition(rules_condition('registration_commerce_item_can_register', array(
    'data:select' => 'commerce-line-item',
  ))->negate())
    ->action('data_set', array(
    'data:select' => 'commerce-line-item:commerce-unit-price:amount',
  ));

  $rules['registration_commerce_unset_unavailable_registration'] = $rule;

  $rule = rules_reaction_rule();

  $rule->label = t('Assert that no other cart contains this registration product');
  $rule->active = FALSE;
  $rule->weight = 0;
  $rule->tags = array("Commerce Cart", "Registration Commerce");
  $rule
    ->event('commerce_line_item_presave')
    ->condition('entity_has_field', array(
      'entity:select' => 'commerce-line-item',
      'field' => REGISTRATION_COMMERCE_FIELD,
    ))
    ->condition(rules_condition('registration_commerce_registration_has_extant_line_item', array(
      'data:select' => 'commerce-line-item',
    )))
    ->action('drupal_message', array(
      'message' => t("This item cannot be added to your cart: it is already in another user's cart."),
      'type' => 'error',
      'repeat' => FALSE,
    ))
    ->action('entity_delete', array(
      'data:select' => 'commerce-line-item',
    ))
    ->action('redirect', array(
      'url:select' => 'commerce-line-item-unchanged:registration-commerce-reg:url',
    ));

  $rules['rules_assert_registration_product_line_item_uniqueness'] = $rule;

  return $rules;
}

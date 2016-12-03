<?php

/**
 * Build the "request content" form that is displayed on content of type `rsc_library_request`.
 * @param $form
 * @param $form_state
 * @param $nid The node id of the content being requested
 * @param $uid The user requesting the content. May be anonymous (0).
 * @return array The form array, according to https://api.drupal.org/api/drupal/developer!topics!forms_api_reference.html/7.x/
 */
function rsc_library_form_request_content($form, &$form_state, $nid) {

  global $user;

  $form['mail'] = array(
    '#type' => 'textfield',
    '#title' => t('Your e-mail address'),
    '#maxlength' => 255,
    '#default_value' => $user->uid ? $user->mail : '',
    '#required' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Request content'),
  );

  return $form;

}


/**
 * Validate the "request content" form
 * @param $form
 * @param $form_state
 */
function rsc_library_form_request_content_validate($form, &$form_state) {
  if (!valid_email_address($form_state['values']['mail'])) {
    form_set_error('mail', t('You must enter a valid e-mail address.'));
  }
}


/**
 * Process the values submitted using the "request content" form
 * @param $form
 * @param $form_state
 */
function rsc_library_form_request_content_submit($form, &$form_state) {

}

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

  global $user;

  $uid = (int) $user->uid;
  $nid = (int) $form_state['build_info']['args'][0];
  $email = $form_state['values']['mail'];

  // Try to retrieve a request ID for this nid & mail
  $rid = db_query("select rid from rsc_library_request where nid=:nid and email=:email", array(
    ':nid' => $nid,
    ':email' => $email
  ))->fetchField(0);

  if (empty($rid)) {
    // Try to create a request ID for the nid & mail
    $rid = db_insert('rsc_library_request')->fields(array(
      'nid' => $nid,
      'email' => $email,
    ))->execute();

    if (empty($rid)) {
      drupal_set_message(t("Failed to submit request"), 'error', FALSE);
      return;
    }
  }

  // check if an existing request is pending

  // Find the last action that was performed on this request ID
  $last_action = db_query("select action from rsc_library_request_log where rid=:rid order by timestamp desc limit 1", array(
    ':rid' => $rid,
  ))->fetchField(0);

  if ($last_action == 'request') {

    drupal_set_message(t("You have already requested this content. Ticket number: @n", array(
      '@n' => $rid,
    )));
    return;

  }

  // log request action
  $rlid = db_insert('rsc_library_request_log')->fields(array(
    'rid'       => $rid,
    'action'    => 'request',
    'user'      => $uid,
    'timestamp' => REQUEST_TIME,
  ))->execute();

  if (!$rlid) {
    drupal_set_message(t("Failed to submit request"), 'error', FALSE);
    return;
  }

  drupal_set_message(t("Your request has been logged. Ticket number: @n", array(
    '@n' => $rid,
  )));

  // Notify administrator
  $to_email = variable_get('rsc_library_request_notify_email');
  $from_email = variable_get('site_mail');

  if (!valid_email_address($to_email)) {
    watchdog('rsc_library', "Email address for notifications about new requests has not been configured.");
    return;
  }

  if (!valid_email_address($from_email)) {
    watchdog('rsc_library', "Site email address has not been configured.");
    return;
  }

  drupal_mail(
    'rsc_library',
    'new_request',
    $to_email,
    user_preferred_language($user),
    array(
      'user_email' => $email,
      'nid' => $nid,
      'date' => strftime("%Y%m%d", REQUEST_TIME),
    ),
    $from_email
  );

}


/**
 * Form builder for the "pending requests" page
 */
function rsc_library_form_pending_requests($form, &$form_state) {

  $result = db_query(<<<EOQ
select *
from rsc_library_request_log as l
inner join (select rlid, max(l.timestamp) as t from rsc_library_request_log l group by rid) as x on l.rlid=x.rlid
inner join rsc_library_request as r on r.rid=l.rid
EOQ
  );

  $header = array(
    'user'  => t('User'),
    'email' => t('Email address'),
    'nid'   => t("Requested node"),
    'time'  => t("Time stamp"),
  );

  $options = array();

  foreach ($result as $record) {
    $options[] = array(
      'user'  => $record->user ? l($record->user, 'user/'.$record->user) : "anon.",
      'email' => $record->email,
      'nid'   => l($record->nid, 'node/'.$record->nid),
      'time'  => strftime("%Y-%m-%d %H:%M:%S", $record->t),
    );
  }

  $form['pending'] = array(
    '#title' => t("Pending requests"),
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $options,
  );

  $form['actions'] = array(
    'accept' => array(
      '#prefix' => '<p>',
      '#type' => 'submit',
      '#value' => t("Accept"),
      '#suffix' => t("Send the requested content to the respective users.").'</p>',
    ),
    'ignore' => array(
      '#prefix' => '<p>',
      '#type' => 'submit',
      '#value' => t("Ignore"),
      '#suffix' => t("Ignore the request. Don't do anything else.").'</p>',
    ),
    'reject' => array(
      '#prefix' => '<p>',
      '#type' => 'submit',
      '#value' => t("Reject"),
      '#suffix' => t("Notify the user that their request was rejected.").'</p>',
    ),
  );

  return $form;

}


function rsc_library_form_pending_requests_validate($form, &$form_state) {

}


function rsc_library_form_pending_requests_submit($form, &$form_state) {
  dpm($form_state);
}

